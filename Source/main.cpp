#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>

#include "flash/flash.h"
using namespace sc::flash;

#include "core/console/console.h"


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
	fs::path executable = argv[0];
	fs::path executable_name = executable.stem();

	// Arguments
	sc::ArgumentParser parser(executable_name.string(), "Tool for downgrading Supercell Flash files");

	parser.add_argument("input")
		.help("Input .sc file")
		.required();

	parser.add_argument("output")
		.help("Name for output .sc file")
		.required();

	parser.add_argument("version")
		.help("Target version for file. Possible values: 1.0 or 0.5")
		.scan<'g', float>()
		.default_value(0.0f);

	try {
		parser.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cout << parser << std::endl;

		std::cout << "Error! " << err.what() << std::endl;
		return 1;
	}

	if (parser["--help"] == true || argc == 1)
	{
		std::cout << parser << std::endl;
		return 0;
	}

	float version = parser.get<float>("version");
	fs::path input(parser.get<std::string>("input"));
	fs::path output(parser.get<std::string>("output"));

	if (!fs::exists(input))
	{
		std::cout << "Input file does not exist" << std::endl;
		return 1;
	}

	if (version != 1.0f && version != 0.5f && version != 0.0f)
	{
		std::cout << "Incorrect target version!" << std::endl;
		return 0;
	}

	bool is_sc2 = is_sc2_file(input);

	if (is_sc2 && version == 0.f)
	{
		version = 1.0f;
	}
	else
	{
		version = 0.5f;
	}



	SupercellSWF swf;
	swf.load(input);
	
	std::cout << "Downgrading from Sc2 to Sc1..." << std::endl;

	// Sc2 has a little bit different vertices order
	// Sort in sc1 order

	if (is_sc2)
	{
		for (Shape& shape : swf.shapes)
		{
			for (ShapeDrawBitmapCommand& command : shape.commands)
			{
				ShapeDrawBitmapCommandVertexArray vertices = command.vertices;
				ShapeDrawBitmapCommandTrianglesArray indices;
				indices.reserve(vertices.size());
	
				indices.push_back(0);
				for (uint16_t i = 1; i < floor((float)vertices.size() / 2) * 2; i += 2) {
					indices.push_back(i);
				}
				for (uint16_t i = (uint16_t)floor(((float)vertices.size() - 1) / 2) * 2; i > 0; i -= 2) {
					indices.push_back(i);
				}
	
				for (uint16_t i = 0; vertices.size() > i; i++)
				{
					command.vertices[i] = vertices[indices[i]];
				}
			}
		}
	}
	
	if (version == 0.5f)
	{
		std::cout << "Downgrading from Sc1 to Sc1 v0.5..." << std::endl;

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
		std::cout << "Saving Sc1 v0.5..." << std::endl;
		swf.save(output, Signature::Lzma);
	}
	else 
	{
		std::cout << "Saving Sc1..." << std::endl;
		swf.save(output, Signature::Zstandard);
	}

	std::cout << "Success" << std::endl;
	return 0;
}