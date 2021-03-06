#pragma once
#include "macros.h"

/* 
 * Helper class to sample a blend shape weight plugs 
 * Parts of this code or borrowed from the Maya to OGRE exporter.
 */
class MeshBlendShapeWeights
{
public:
	MeshBlendShapeWeights(const MPlug& weightArrayPlug);
	~MeshBlendShapeWeights();

	auto numWeights() const { return m_originalWeightPlugStates.size(); }
	MPlug getWeightPlug(const int index) const { return m_weightArrayPlug[index]; }

	double getOriginalWeight(const int index) const { return m_originalWeightPlugStates.at(index).weight; }

	void clearWeightsExceptFor(const size_t index) const;
	void breakConnections();

private:
	DISALLOW_COPY_MOVE_ASSIGN(MeshBlendShapeWeights);

	struct OriginalWeightPlugState
	{
		double weight;
		bool locked;
		MPlugArray srcConnections;
		MPlugArray dstConnections;
	};

	typedef std::vector<OriginalWeightPlugState> OriginalWeightPlugStates;

	MPlug m_weightArrayPlug;
	OriginalWeightPlugStates m_originalWeightPlugStates;
};

