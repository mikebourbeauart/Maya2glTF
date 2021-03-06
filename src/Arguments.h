#pragma once

#include "IndentableStream.h"
#include "sceneTypes.h"

class SyntaxFactory : MSyntax
{
public:
	SyntaxFactory();
	~SyntaxFactory();

	const char* usage() const
	{
		const auto result = m_usage.c_str();
		return result;
	}

	const char* longArgName(const char* shortName) const
	{
		const auto longName = m_argNames.at(shortName);
		return longName;
	}

	static const SyntaxFactory& get();
	static MSyntax createSyntax();

private:
	DISALLOW_COPY_MOVE_ASSIGN(SyntaxFactory);

	std::map<const char*, const char*> m_argNames;
	std::string m_usage;

	void registerFlag(std::stringstream& ss, const char *shortName, const char *longName, bool isMultiUse, MArgType argType1 = kNoArg);
	void registerFlag(std::stringstream& ss, const char *shortName, const char *longName, MArgType argType1 = kNoArg);
};

struct AnimClipArg
{
	AnimClipArg(std::string name, const MTime& startTime, const MTime& endTime, const double framesPerSecond)
		: name{std::move(name)}
		, startTime{ startTime }
		, endTime{ endTime }
		, framesPerSecond{ framesPerSecond }
	{
	}

	DEFAULT_COPY_MOVE_ASSIGN_DTOR(AnimClipArg);

	std::string name;
	MTime startTime;
	MTime endTime;
	double framesPerSecond;

	MTime duration() const
	{
		return endTime - startTime;
	}

	int frameCount() const
	{
		return static_cast<int>(ceil(duration().as(MTime::kSeconds) * framesPerSecond));
	}
};

struct DagPathComparer
{
	bool operator ()(const MDagPath& a, const MDagPath& b) const
	{
		return strcmp(a.fullPathName().asChar(), b.fullPathName().asChar()) < 0;
	}
};

typedef std::set<MDagPath, DagPathComparer> Selection;

class Arguments
{
public:
	Arguments(const MArgList& args, const MSyntax& syntax);
	~Arguments();

	MString sceneName;
	MString outputFolder;
	Selection selection;

	/* The extension to use for glTF files. Some viewers require lower-case gltf, others might need the official glTF */
	MString gltfFileExtension = "gltf";

	/* The extension to use for glb files. */
	MString glbFileExtension = "glb";

	/** Outputs a single binary GLB file */
	bool glb = false;

	/** Create a default material for primitives that don't have shading in Maya? */
	bool defaultMaterial = false;

	/** Assign a color with a different hue to each material, for debugging purposes */
	bool colorizeMaterials = false;

	/** Skip all non PBR materials. By default these are converted to PBR materials */
	bool skipStandardMaterials = false;

	/** Always use 32-bit indices, even when 16-bit would be sufficient */
	bool force32bitIndices = false;

	/** If non-null, dump the Maya intermediate objects to the stream */
	IndentableStream* dumpMaya;

	/** If non-null, dump the GLTF JSON to the stream */
	IndentableStream* dumpGLTF;

	/** Embed all resources in the GLTF file? By default a separate GLTF JSON and binary BIN file is exported */
	bool embedded = false;

	/** By default the Maya node names are assigned to the GLTF node names */
	bool disableNameAssignment = false;

	/** Generate debug tangent vector lines? */
	bool debugTangentVectors = false;

	/** Generate debug normal vector lines? */
	bool debugNormalVectors = false;

	/** The length of the debugging vectors */
	float debugVectorLength = 0.1f;

	/** When non-0, instead of using Maya's tangents, use tangents as computed in Morten Mikkelsen's thesis http://image.diku.dk/projects/media/morten.mikkelsen.08.pdf*/
	float mikkelsenTangentAngularThreshold = 0;

	/** The scale factor to apply to the vertex positions */
	float globalScaleFactor = 1;

	/** The opacity factor to apply to the material */
	float opacityFactor = 1;

	/** The semantics (aka glTF attributes) that should be exported for the mains. Defaults to all semantics contained in the mesh */
	MeshSemanticSet meshPrimitiveAttributes;

	/** The semantics (aka glTF attributes) that should be exported for blend shapes. Defaults to NORMAL and TANGENT */
	MeshSemanticSet blendPrimitiveAttributes;

	/** Exclude TEXCOORD semantics (aka glTF attributes) when the mesh primitive doesn't have textures? By default TEXCOORD attributes are always included */
	bool excludeUnusedTexcoord = false;

	/** Ignore all skin clusters */
	bool skipSkinClusters = false;

	/** Ignore all blend shapes */
	bool skipBlendShapes = false;

	/** Ignore these mesh deformers. By default the deformer closest to the displayed mesh is used. */
	MSelectionList ignoreMeshDeformers;

	/** Ignore segment scale compensation */
	bool ignoreSegmentScaleCompensation = false;

	/** Keep GLTF shape nodes that are the only child of their parent transform node */
	bool keepShapeNodes = false;

	/** Bake scaling factor by scaling vertices and positions? By default a root scaling node is added instead */
	bool bakeScalingFactor = false;

	/** Force the creation of a root node, even when the scaling factor is 1, in which case an extra root node is not needed*/
	bool forceRootNode = false;

	/** 
	 * The time where the 'initial values' of all nodes are to be found (aka neutral base pose) 
	 * By default the current time is used, unless animation is used, then frame 0 is used.
	 */
	MTime initialValuesTime;

	/** Redraw the viewport while exporting? True in debug builds by default, false otherwise (for speed) */
#ifdef _DEBUG
	bool redrawViewport = true;
#else
	bool redrawViewport = false;
#endif

	/** 
	 * Only export the directly selected nodes only, not the descendants of these 
	 * By default all descendants are exported too.
	 */
	bool selectedNodesOnly = false;

	std::vector<AnimClipArg> animationClips;

	/** Copyright text of the exported file */
	MString copyright;

	void assignName(GLTF::Object& glObj, const std::string& name) const
	{
		if (!disableNameAssignment)
		{
			glObj.name = name;
		}
	}

	float getBakeScaleFactor() const { return bakeScalingFactor ? globalScaleFactor : 1; }
	float getRootScaleFactor() const { return bakeScalingFactor ? 1 : globalScaleFactor; }

private:
	DISALLOW_COPY_MOVE_ASSIGN(Arguments);

	static void select(Selection& selection, const MDagPath& dagPath, bool includeDescendants);

	std::ofstream m_mayaOutputFileStream;
	std::ofstream m_gltfOutputFileStream;

	std::unique_ptr<IndentableStream> m_mayaOutputStream;
	std::unique_ptr<IndentableStream> m_gltfOutputStream;
};

