#include "externals.h"
#include "ExportableResources.h"
#include "DagHelper.h"
#include "MayaException.h"
#include "ExportableMaterial.h"

ExportableResources::ExportableResources(const Arguments& args)
	:m_args(args)
{
}

ExportableResources::~ExportableResources()
{
}

ExportableMaterial* ExportableResources::getMaterial(const MObject& shaderGroup)
{
	MStatus status;

	if (shaderGroup.isNull())
		return nullptr;

	MObject surfaceShader = DagHelper::findSourceNodeConnectedTo(shaderGroup, "surfaceShader");

	if (surfaceShader.isNull())
		return nullptr;

	MFnDependencyNode shaderNode(surfaceShader, &status);

	MUuid mayaId = shaderNode.uuid(&status);
	THROW_ON_FAILURE(status);

	const std::string key(mayaId.asString().asChar());

	auto& materialPtr = m_materialMap[key];
	if (materialPtr)
	{
		cout << prefix << "Reusing material instance " << key << endl;
	}
	else
	{
		// Create new material.
		materialPtr = ExportableMaterial::from(*this, shaderNode);
	}

	return materialPtr.get();
}

GLTF::Image* ExportableResources::getImage(const char* path)
{
	if (!std::experimental::filesystem::exists(path)) 
	{
		MayaException::printError(formatted("Image with path '%s' does not exist!", path));
		MayaException::printError("(it is adviced to use a Maya project and relative paths)");
		return nullptr;
	}

	std::string key(path);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	auto& imagePtr = m_imageMap[key];
	if (!imagePtr)
	{
		try
		{
			imagePtr.reset(GLTF::Image::load(path));
		}
		catch (std::exception& ex)
		{
			MayaException::printError(formatted("Failed to load image '%s': %s", path, ex.what()));
		}
	}
	return imagePtr.get();
}

static GLTF::Constants::WebGL getSamplerWrapping(const ImageTilingFlags tiling)
{
	// TODO: Verify mapping
	switch (tiling)
	{
	case IMAGE_TILING_Wrap:
		return GLTF::Constants::WebGL::REPEAT;
	case IMAGE_TILING_Mirror:
	case IMAGE_TILING_Wrap | IMAGE_TILING_Mirror:
		return GLTF::Constants::WebGL::MIRRORED_REPEAT;
	default:
		return GLTF::Constants::WebGL::CLAMP_TO_EDGE;
	}
}

/*
From the Maya documentation:

For Viewport 2.0, the filter types supported are as follows:

Off:
		No filter.
		No mipmaps are built and point sampling is used.
		Your resulting image may be pixelated.

Box:
		Point sampling is performed on mipmap textures.
		Mipmapped textures are used when zooming away from an object
		so that textures appear less noisy when zoomed out.

Mipmap, Quadratic, Quartic, Gaussian:
		Trilinear filtering is used and sampling is performed between mipmaps.
		This option provides the smoothest results and produces smooth results
		when the object is zoomed out.
*/
static GLTF::Constants::WebGL getSamplerMinFilter(const ImageFilterKind filter)
{
	switch (filter) {
	case IMAGE_FILTER_Off: 
		return GLTF::Constants::WebGL::NEAREST;
	case IMAGE_FILTER_Box: 
		return GLTF::Constants::WebGL::LINEAR_MIPMAP_NEAREST;
	default:;
		return GLTF::Constants::WebGL::LINEAR_MIPMAP_LINEAR;
	}
}
static GLTF::Constants::WebGL getSamplerMagFilter(const ImageFilterKind filter)
{
	switch (filter) {
	case IMAGE_FILTER_Off:
	case IMAGE_FILTER_Box:
		return GLTF::Constants::WebGL::NEAREST;
	default:;
		return GLTF::Constants::WebGL::LINEAR;
	}
}

GLTF::Sampler* ExportableResources::getSampler(
	const ImageFilterKind filter,
	const ImageTilingFlags uTiling,
	const ImageTilingFlags vTiling)
{
	const auto key = (filter << 8) | (vTiling << 4) | uTiling;

	auto& samplerPtr = m_samplerMap[key];
	if (!samplerPtr)
	{
		samplerPtr = std::make_unique<GLTF::Sampler>();
		samplerPtr->wrapS = getSamplerWrapping(uTiling);
		samplerPtr->wrapT = getSamplerWrapping(vTiling);
		samplerPtr->magFilter = getSamplerMagFilter(filter);
		samplerPtr->minFilter = getSamplerMinFilter(filter);
	}
	return samplerPtr.get();
}

GLTF::Texture* ExportableResources::getTexture(GLTF::Image* image, GLTF::Sampler* sampler)
{
	const auto key = std::make_pair(image, sampler);

	auto& texturePtr = m_TextureMap[key];
	if (!texturePtr)
	{
		texturePtr = std::make_unique<GLTF::Texture>();
		texturePtr->source = image;
		texturePtr->sampler = sampler;
	}
	return texturePtr.get();
}

ExportableMaterial* ExportableResources::getDebugMaterial(const Float3& hsv)
{
	auto& materialPtr = m_debugMaterialMap[hsv];
	if (!materialPtr)
	{
		materialPtr = std::make_unique<ExportableDebugMaterial>(hsv);
	}

	return materialPtr.get();
}
