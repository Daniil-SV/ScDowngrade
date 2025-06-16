#include <filesystem>
namespace fs = std::filesystem;

#include <iostream>

#include "flash/flash.h"
using namespace sc::flash;

#include "core/console/console.h"

void downgrade(const fs::path& input, const fs::path& output, float target)
{
	bool is_sc2 = false;
	{
		wk::InputFileStream file(input);
		is_sc2 = SupercellSWF::IsSC2(file);
	}

	if (is_sc2 && target == 0.f)
	{
		target = 1.0f;
	}
	else
	{
		target = 0.5f;
	}

	SupercellSWF swf;
	swf.load(input);

	std::cout << "Downgrading from Sc2 to Sc1..." << std::endl;

	// Common stuff converting
	if (is_sc2)
	{
		// Sc2 has a little bit different vertices order
		// Sort in sc1 order
		for (Shape& shape : swf.shapes)
		{
			for (ShapeDrawBitmapCommand& command : shape.commands)
			{
				command.sort_advanced_vertices(false);
			}
		}
	}

	if (target == 0.5f)
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

		swf.use_external_textures = false;
		swf.save_custom_property = false;
		swf.use_multi_resolution = false;
		swf.use_low_resolution = false;
		std::cout << "Saving Sc1 v0.5..." << std::endl;
		swf.save(output, Signature::Lzma);
	}
	else
	{
		std::cout << "Saving Sc1..." << std::endl;
		for (SWFTexture& texture : swf.textures)
		{
			// decompress sctx to raw to avoid unsuported encoding exception in texture tag selector function
			if (texture.encoding() == SWFTexture::TextureEncoding::SupercellTexture)
			{
				texture.encoding(SWFTexture::TextureEncoding::Raw);
			}
		}

		swf.use_external_textures = false;
		swf.save(output, Signature::Zstandard);
	}
}

int main(int argc, char* argv[])
{
	fs::path executable = argv[0];
	fs::path executable_name = executable.stem();

	// Arguments
	wk::ArgumentParser parser(executable_name.string(), "Tool for downgrading Supercell Flash files");

	parser.add_argument("input")
		.help("Input .sc file or folder with .sc files")
		.required();

	parser.add_argument("output")
		.help("Name for output .sc file")
		.required();

	parser.add_argument("version")
		.help("Target version for file. Possible values: 2.0, 1.0, 0.5")
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

	try
	{
		if (fs::is_regular_file(input))
		{
			downgrade(input, output, version);
		}
		else if (fs::is_directory(input))
		{
			fs::create_directories(output);

			for (const auto& entry : fs::directory_iterator(input))
			{
				fs::path path = entry.path();

				if (fs::is_regular_file(path) && path.extension() == ".sc")
				{
					std::cout << "Working on " << path << std::endl;
					fs::path basename = path.filename();
					fs::path file_output = output / basename;
					downgrade(path, file_output, version);
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}
	

	std::cout << "Success" << std::endl;
	return 0;
}