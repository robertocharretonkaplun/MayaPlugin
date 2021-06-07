
#include <maya/MPxNode.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MDistance.h>
#include <maya/MIntArray.h>
#include <maya/MDagPath.h>

#include <maya/MIOStream.h>


class ExportComand : public MPxCommand {
public:
	ExportComand() = default;
	~ExportComand() = default;
	virtual MStatus	doIt(const MArgList&);
	static void* creator();
	void extractVertices();
	void extractIndex();
};

MStatus ExportComand::doIt(const MArgList&)
{
	extractIndex();
	extractVertices();
	return MS::kSuccess;
}

void* ExportComand::creator()
{
	return new ExportComand;
}

void ExportComand::extractVertices()
{
	// we assume here that Maya has been initialized and the file in
	// question has already been loaded.

	MStatus stat;
	MItDag dagIter(MItDag::kBreadthFirst, MFn::kInvalid, &stat);

	for (; !dagIter.isDone(); dagIter.next())
	{
		MDagPath dagPath;
		stat = dagIter.getPath(dagPath);

		if (stat)
		{
			MFnDagNode dagNode(dagPath, &stat);

			// this object cannot be intermediate, and it must be a mesh
			// and it can't be a transform.
			// Intermediate objects are special meshes
			// that are not drawn used for mesh morphs or something.
			if (dagNode.isIntermediateObject()) continue;
			if (!dagPath.hasFn(MFn::kMesh)) continue;
			if (dagPath.hasFn(MFn::kTransform)) continue;

			MFnMesh fnMesh(dagPath);

			// get the vertices that are part of the current mesh
			MPointArray vertexList;
			MFloatVectorArray normalList;
			MFloatVectorArray tangentList;
			MFloatVectorArray biNorList;
			MFloatArray uArray, vArray;

			fnMesh.getPoints(vertexList, MSpace::kWorld);
			fnMesh.getNormals(normalList, MSpace::kWorld);
			fnMesh.getTangents(tangentList, MSpace::kWorld);
			fnMesh.getBinormals(biNorList, MSpace::kWorld);
			fnMesh.getUVs(uArray, vArray);
			//fnMesh.gettangen
			//fnMesh.getnormal
			//fnMesh.tanget
			int vertexLength = vertexList.length();
			int normalLength = normalList.length();
			int tangentLength = tangentList.length();
			int biNorLength = biNorList.length();
			// iterate through all the vertices
			for (INT32 i = 0; i < vertexList.length(); i++)
			{
				vertexList[i].cartesianize();
				MPoint point = vertexList[i];

				// here is your data... now go do whatever you want with
				// it. if you need a unique identifier for this vertex,
				// use it's index in the mesh, and some kind of mesh id.
				// these stay constant while exporting ( so long as the file is
				// not edited )
				(point.x, point.y, point.z);
			}
		}
	}
}

void ExportComand::extractIndex() {
	MStatus stat;
	MItDag dagIter(MItDag::kBreadthFirst, MFn::kInvalid, &stat);

	for (; !dagIter.isDone(); dagIter.next())
	{
		MDagPath dagPath;
		stat = dagIter.getPath(dagPath);

		if (stat)
		{
			MFnDagNode dagNode(dagPath, &stat);

			if (dagNode.isIntermediateObject()) continue;
			if (!dagPath.hasFn(MFn::kMesh)) continue;
			if (dagPath.hasFn(MFn::kTransform)) continue;

			// get the mesh and all its vertices
			MFnMesh fnMesh(dagPath);
			MPointArray vertexList;
			fnMesh.getPoints(vertexList, MSpace::kWorld);

			MObject comp;
			// now iterate over all the polygons in the mesh
			MItMeshPolygon piter(dagPath, comp);
			for (; !piter.isDone(); piter.next())
			{
				// for each polygon you can get the indices of
				// each of its vertices in the vertex list above
				MIntArray vertexIdxs;
				piter.getVertices(vertexIdxs);

				if (vertexIdxs.length() == 3)
				{
					// process a triangle
					MPoint point0 = vertexList[vertexIdxs[0]];
					MPoint point1 = vertexList[vertexIdxs[1]];
					MPoint point2 = vertexList[vertexIdxs[2]];

					//processTriangle(point0, point1, point2);
				}
			}
		}
	}
}


MStatus initializePlugin(MObject _obj)
{
	MFnPlugin	plugin(_obj, "Autodesk", "2020");
	MStatus		stat;
	stat = plugin.registerCommand("ExportComand",
		ExportComand::creator);
	if (!stat)
		stat.perror("registerCommand");
	return stat;
}

MStatus uninitializePlugin(MObject _obj)
{
	MFnPlugin	plugin(_obj);
	MStatus		stat;
	stat = plugin.deregisterCommand("ExportComand");
	if (!stat)
		stat.perror("deregisterCommand");
	return stat;
}


