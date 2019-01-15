#include <iostream>
#include <fstream>
#include <memory>

#include "draco/mesh/mesh.h"
#include "draco/point_cloud/point_cloud.h"
#include "draco/core/vector_d.h"
#include "draco/io/mesh_io.h"

template<int N>
void printVec(draco::VectorD<float, N> &v) {
	for (int i = 0; i < N; i++) {
		std::printf("%f ", v[i]);
	}
	std::printf("\n");
}

template<int N>
void printAttribute(const char *label, const draco::PointAttribute *attr) {
	std::printf("%s:\n", label);
	if (attr != nullptr) {
		for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
			draco::VectorD<float, N> p;
			attr->GetValue(i, &p[0]);
			std::printf(" ");
			printVec(p);
		}
	}
	else {
		std::printf(" none\n");
	}
	std::printf("\n");
}

void dump(const draco::Mesh &mesh) {
	printAttribute<3>("Positions", mesh.GetNamedAttribute(draco::GeometryAttribute::POSITION));
	printAttribute<3>("Normals", mesh.GetNamedAttribute(draco::GeometryAttribute::NORMAL));
	printAttribute<3>("Colors", mesh.GetNamedAttribute(draco::GeometryAttribute::COLOR));
	printAttribute<2>("Texture coordinates", mesh.GetNamedAttribute(draco::GeometryAttribute::TEX_COORD));

	std::printf("Faces:");
	for (int i = 0; i < mesh.num_faces(); i++) {
		const draco::Mesh::Face face = mesh.face(draco::FaceIndex(i));
		std::printf(" #%d:  %d %d %d\n", i, face[0], face[1], face[2]);
	}
}

void test() {
	auto mesh = draco::ReadMeshFromFile("mesh.drc").value();
	std::printf("Loaded from disk:\n");
	dump(*mesh.get());
}

int main()
{
	float positions[][3] = {
		{ 1, 2, 3 },
		{ 4, 5, 6 },
		{ 7, 8, 9 },
	};

	float normals[][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 },
	};

	draco::DataBuffer position_buffer;
	draco::GeometryAttribute position_attr;
	position_attr.Init(
		draco::GeometryAttribute::Type::POSITION,
		&position_buffer,
		3,
		draco::DataType::DT_FLOAT32,
		false,
		sizeof(float) * 3,
		0
	);

	draco::DataBuffer normal_buffer;
	draco::GeometryAttribute normal_attr;
	normal_attr.Init(
		draco::GeometryAttribute::Type::NORMAL,
		&normal_buffer,
		3,
		draco::DataType::DT_FLOAT32,
		false,
		sizeof(float) * 3,
		0
	);

	draco::Mesh mesh;

	const int position_attr_id = mesh.AddAttribute(position_attr, true, 3);
	mesh.attribute(position_attr_id)->SetAttributeValue(draco::AttributeValueIndex(0), positions[0]);
	mesh.attribute(position_attr_id)->SetAttributeValue(draco::AttributeValueIndex(1), positions[1]);
	mesh.attribute(position_attr_id)->SetAttributeValue(draco::AttributeValueIndex(2), positions[2]);

	const int normal_attr_id = mesh.AddAttribute(normal_attr, true, 3);
	mesh.attribute(normal_attr_id)->SetAttributeValue(draco::AttributeValueIndex(0), normals[0]);
	mesh.attribute(normal_attr_id)->SetAttributeValue(draco::AttributeValueIndex(1), normals[1]);
	mesh.attribute(normal_attr_id)->SetAttributeValue(draco::AttributeValueIndex(2), normals[2]);

	mesh.AddFace({ draco::PointIndex(0), draco::PointIndex(1), draco::PointIndex(2) });

	dump(mesh);

	{
		auto stream = std::ofstream("mesh.drc", std::ios::binary);
		draco::WriteMeshIntoStream(&mesh, stream);
	}

	test();
}
