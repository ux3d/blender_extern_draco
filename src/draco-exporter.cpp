#include <iostream>
#include <fstream>
#include <memory>

#include "draco/mesh/mesh.h"
#include "draco/point_cloud/point_cloud.h"
#include "draco/core/vector_d.h"
#include "draco/io/mesh_io.h"

#if defined(_MSC_VER)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

enum {
	POSITION, NORMAL, NUM_ATTRIBUTES
};

struct DracoExporter {
	draco::Mesh mesh;
	draco::DataBuffer buffers[NUM_ATTRIBUTES];
	int attribute_ids[NUM_ATTRIBUTES];
	int counters[NUM_ATTRIBUTES];
};

draco::GeometryAttribute createAttribute(
	draco::GeometryAttribute::Type type,
	draco::DataBuffer &buffer,
	int components
) {
	draco::GeometryAttribute attribute;
	attribute.Init(
		type,
		&buffer,
		components,
		draco::DataType::DT_FLOAT32,
		false,
		sizeof(float) * components,
		0
	);
	return attribute;
}

extern "C" {

	DLL_EXPORT DracoExporter * __cdecl createExporter(int num_vertices) {
		auto exporter = new DracoExporter;

		for (int i = 0; i < NUM_ATTRIBUTES; i++) {
			exporter->counters[i] = 0;
		}

		draco::GeometryAttribute position_attribute = createAttribute(
			draco::GeometryAttribute::POSITION,
			exporter->buffers[POSITION],
			3
		);

		draco::GeometryAttribute normal_attribute = createAttribute(
			draco::GeometryAttribute::NORMAL,
			exporter->buffers[NORMAL],
			3
		);

		exporter->attribute_ids[POSITION] =
			exporter->mesh.AddAttribute(position_attribute, false, num_vertices);

		exporter->attribute_ids[NORMAL] =
			exporter->mesh.AddAttribute(normal_attribute, false, num_vertices);

		// Force identity mapping between vertex index and attribute indices.
		// A face referencing the vertex #0 references in fact the position #0, normal #0, ...
		exporter->mesh.attribute(exporter->attribute_ids[POSITION])->SetIdentityMapping();
		exporter->mesh.attribute(exporter->attribute_ids[NORMAL])->SetIdentityMapping();

		return exporter;
	}

	DLL_EXPORT void __cdecl disposeExporter(DracoExporter *exporter) {
		delete exporter;
	}

	DLL_EXPORT void __cdecl addPosition(
		DracoExporter *exporter,
		float x,
		float y,
		float z
	) {
		float data[] = { x, y, z };
		exporter->mesh.attribute(exporter->attribute_ids[POSITION])->SetAttributeValue(
			draco::AttributeValueIndex(exporter->counters[POSITION]++),
			data
		);
	}

	DLL_EXPORT void __cdecl addNormal(
		DracoExporter *exporter,
		float x,
		float y,
		float z
	) {
		float data[] = { x, y, z };
		exporter->mesh.attribute(exporter->attribute_ids[NORMAL])->SetAttributeValue(
			draco::AttributeValueIndex(exporter->counters[NORMAL]++),
			data
		);
	}

	DLL_EXPORT void __cdecl addFace(
		DracoExporter *exporter,
		int a,
		int b,
		int c
	) {
		exporter->mesh.AddFace({ draco::PointIndex(a), draco::PointIndex(b), draco::PointIndex(c) });
	}

	/// Wrap Python strings in c_char_p().
	DLL_EXPORT void __cdecl exportMesh(DracoExporter *exporter, char *fileName) {
		draco::DataBuffer *b = exporter->mesh.attribute(exporter->attribute_ids[POSITION])->buffer();
		auto stream = std::ofstream(fileName, std::ios::binary);
		draco::WriteMeshIntoStream(&exporter->mesh, stream);
	}

} // extern "C"
