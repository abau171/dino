#pragma once

#include <string>
#include <vector>

#include "common.h"
#include "geometry.h"
#include "obj.h"
#include "bvh.h"

struct camera_t {

	float phi, theta;
	vec3 position, forward, up, right;
	float aspect_ratio;

	void set_rotation(float new_phi, float new_theta) {

		phi = new_phi;
		theta = new_theta;

		forward = {0.0f, 0.0f, -1.0f};
		forward = mat4_rotate_y(phi) * (mat4_rotate_x(theta) * forward);
		forward.normalize();
		up = {0.0f, 1.0f, 0.0f};
		right = forward.cross(up);
		right.normalize();
		up = right.cross(forward);

	}

	void rotate(float d_phi, float d_theta) {

		set_rotation(phi + d_phi, theta + d_theta);

	}

};

struct surface_t {
	float specular_weight, transmission_weight; // first checks for specular reflection, if not then checks for transmission vs diffuse
	float spec_power; // spec_power=0 means perfect reflection
	color3 diffuse, specular, emission;
};

struct volume_t {
	float refractive_index, scatter, scatter_g;
	color3 attenuation;
};

struct material_t {
	surface_t surface;
	volume_t volume;
};

struct sphere_instance_t {
	sphere_t shape;
	material_t material;
};

struct model_t {
	int tri_start, tri_end;
	bvh_node_t* bvh;
};

struct instance_t {
	int model_index;
	mat4 transform;
	mat4 inv_transform;
	material_t material;
};

struct scene_parameters_t {
	volume_t air_volume;
	color3 background_emission;
	float aperture_radius, focal_distance;
};

struct scene_t {

	scene_parameters_t params;

	material_t* last_material;
	mat4* last_transform;
	mat4* last_inv_transform;

	std::vector<sphere_instance_t> spheres;
	std::vector<triangle_t> triangles;
	std::vector<model_t> models;
	std::vector<std::vector<bvh_node_t>> bvhs;
	std::vector<instance_t> instances;

	void addSphere(vec3 center, float radius) {

		material_t material = {{
				0.0f,
				0.0f,
				0.0f,
				{0.0f, 0.0f, 0.0f},
				{1.0f, 1.0f, 1.0f},
				{0.0f, 0.0f, 0.0f}
			}, {
				1.0f,
				0.0f,
				0.0f,
				{1.0f, 1.0f, 1.0f}
			}};

		spheres.push_back({
			{center, radius},
			material});

		last_material = &spheres.back().material;

	}

	int addModel(std::string filename) {

		int tri_start = (int) triangles.size();

		std::vector<triangle_t> model_triangles = loadObj(filename);

		std::vector<indexed_aabb_t> bounds;
		for (int i = 0; i < model_triangles.size(); i++) {
			indexed_aabb_t bound;
			bound.aabb = model_triangles[i].getBound();
			bound.index = i;
			bounds.push_back(bound);
		}

		int bvh_index = (int) bvhs.size();
		bvhs.push_back(std::vector<bvh_node_t>());
		std::vector<int> indices;
		buildBVH(bounds, bvhs[bvh_index], indices);

		for (int i = 0; i < indices.size(); i++) {
			triangles.push_back(model_triangles[indices[i]]);
		}

		models.push_back({tri_start, tri_start + (int) indices.size()});
		return (int) models.size() - 1;

	}

	void addInstance(int model_index) {

		material_t material = {{
				0.0f,
				0.0f,
				0.0f,
				{0.0f, 0.0f, 0.0f},
				{1.0f, 1.0f, 1.0f},
				{0.0f, 0.0f, 0.0f}
			}, {
				1.0f,
				0.0f,
				0.0f,
				{1.0f, 1.0f, 1.0f}
			}};

		instances.push_back({
			model_index,
			mat4_identity(),
			mat4_identity(),
			material});

		last_material = &instances.back().material;
		last_transform = &instances.back().transform;
		last_inv_transform = &instances.back().inv_transform;

	}

	void setSpecularWeight(float specular_weight) {
		last_material->surface.specular_weight = specular_weight;
	}

	void setTransmissionWeight(float transmission_weight) {
		last_material->surface.transmission_weight = transmission_weight;
	}

	void setSpecularPower(float spec_power) {
		last_material->surface.spec_power = spec_power;
	}

	void setRefractiveIndex(float refractive_index) {
		last_material->volume.refractive_index = refractive_index;
	}

	void setDiffuse(color3 diffuse) {
		last_material->surface.diffuse = diffuse.gammaToLinear();
	}

	void setSpecular(color3 specular) {
		last_material->surface.specular = specular.gammaToLinear();
	}

	void setEmission(color3 emission, float emission_intensity) {
		last_material->surface.emission = emission.gammaToLinear() * emission_intensity;
	}

	void setAttenuation(color3 attenuation) {
		last_material->volume.attenuation = attenuation.gammaToLinear();
	}

	void setScatter(float scatter, float scatter_g = 0.0f) {
		last_material->volume.scatter = scatter;
		last_material->volume.scatter_g = scatter_g;
	}

	void translate(vec3 translation) {
		*last_transform = mat4_translation(translation) * (*last_transform);
		*last_inv_transform = last_transform->invert();
	}

	void scale(float scalar) {
		*last_transform = mat4_scale(scalar) * (*last_transform);
		*last_inv_transform = last_transform->invert();
	}

	void rotate_x(float radians) {
		*last_transform = mat4_rotate_x(radians) * (*last_transform);
		*last_inv_transform = last_transform->invert();
	}

	void rotate_y(float radians) {
		*last_transform = mat4_rotate_y(radians) * (*last_transform);
		*last_inv_transform = last_transform->invert();
	}

	void rotate_z(float radians) {
		*last_transform = mat4_rotate_z(radians) * (*last_transform);
		*last_inv_transform = last_transform->invert();
	}

};
