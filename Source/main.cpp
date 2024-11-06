#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>

#include "flash/flash.h"
using namespace sc::flash;


static bool is_sc2_file(fs::path path)
{
	sc::InputFileStream file(path);

	if (file.read_unsigned_short() == SC_MAGIC && file.read_unsigned_int() == 5)
	{
		return true;
	}

	return false;
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		return 1;
	}

	fs::path input(argv[1]);
	fs::path output(argv[2]);

	if (!fs::exists(input))
	{
		std::cout << "Input file does not exist" << std::endl;
		return 1;
	}

	bool is_sc2 = is_sc2_file(input);

	SupercellSWF swf;
	swf.load(input);
	
	if (is_sc2)
	{
		std::cout << "Downgrading from Sc2 to Sc1..." << std::endl;

		// Sc2 has a little bit different vertices order
		// Sort in sc1 order

		for (Shape& shape : swf.shapes)
		{
			for (ShapeDrawBitmapCommand& command : shape.commands)
			{
				SWFVector<ShapeDrawBitmapCommandVertex> vertices = command.vertices;
				SWFVector<uint16_t> indices;
				indices.reserve(vertices.size());

				indices.push_back(0);
				for (uint16_t i = 1; i < floor((float)vertices.size() / 2) * 2; i += 2) {
					indices.push_back(i);
				}
				for (uint16_t i = floor(((float)vertices.size() - 1) / 2) * 2; i > 0; i -= 2) {
					indices.push_back(i);
				}

				for (uint16_t i = 0; vertices.size() > i; i++)
				{
					command.vertices[i] = vertices[indices[i]];
				}
			}
		}

		std::cout << "Saving sc1..." << std::endl;
		swf.save(output, Signature::Zstandard);
	}
	else
	{
		std::cout << "Downgrading from Sc1 to Sc0.5..." << std::endl;

		for (SWFTexture& texture : swf.textures)
		{
			texture.encoding(SWFTexture::TextureEncoding::Raw);
		}

		for (TextField& textfield : swf.textfields)
		{
			textfield.auto_kern = false;
		}

		swf.use_external_texture_files = false;
		swf.save_custom_property = false;
		swf.use_multi_resolution = false;
		swf.use_low_resolution = false;
		swf.save(output, Signature::Lzma);
	}

	std::cout << "Success" << std::endl;
	return 0;
}