#include "stdafx.h"

void Brush::create() {
	gl->glCreateTextures(GL_TEXTURE_2D, 1, &brush_texture);
	gl->glTextureParameteri(brush_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl->glTextureParameteri(brush_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	set_size(size);

	shader = resource_manager.load<Shader>({ "Data/Shaders/brush.vs", "Data/Shaders/brush.fs" });
}

void Brush::set_position(const glm::vec2& position) {
	const glm::vec2 center_position = (position + brush_offset) - size * granularity * 0.25f;
	this->position = glm::floor(center_position) - glm::ceil(brush_offset);
	if (!uv_offset_locked) {
		uv_offset = glm::abs((center_position - glm::vec2(this->position)) * 4.f);
	}
}

glm::vec2 Brush::get_position() const {
	return glm::vec2(position) + brush_offset + 1.f;
}

void Brush::set_size(const int size) {
	const int change = size - this->size;

	this->size = std::clamp(size, 0, 240);
	const int cells = std::ceil(((this->size * 2 + 1) * granularity + 3) / 4.f);
	brush.clear();
	brush.resize(cells * 4 * cells * 4, { 0, 0, 0, 0 });

	set_shape(shape);

	set_position(glm::vec2(position) + glm::vec2(uv_offset + size * granularity - change * granularity) * 0.25f);
}

void Brush::set_shape(const Shape shape) {
	this->shape = shape;

	const int cells = std::ceil(((this->size * 2 + 1) * granularity + 3) / 4.f);

	for (int i = 0; i < this->size * 2 + 1; i++) {
		for (int j = 0; j < this->size * 2 + 1; j++) {
			for (int k = 0; k < granularity; k++) {
				for (int l = 0; l < granularity; l++) {
					if (contains(i, j)) {
						brush[(j * granularity + l) * cells * 4 + i * granularity + k] = { 0, 255, 0, 128 };
					} else {
						brush[(j * granularity + l) * cells * 4 + i * granularity + k] = { 0, 0, 0, 0 };
					}
				}
			}
		}
	}

	gl->glBindTexture(GL_TEXTURE_2D, brush_texture);
	gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cells * 4, cells * 4, 0, GL_BGRA, GL_UNSIGNED_BYTE, brush.data());
}

/// Whether the brush shape contains the point, Arguments in brush coordinates
bool Brush::contains(const int x, const int y) const {
	const int cells = std::ceil(((size * 2 + 1) * granularity + 3) / 4.f);

	switch (shape) {
		case Shape::square:
			return true;
		case Shape::circle: {
			float distance = (x - size) * (x - size);
			distance += (y - size) * (y - size);
			return distance <= size * size;
		}
		case Shape::diamond:
			return std::abs(x - size) + std::abs(y - size) <= size;
	}
	return true;
}

void Brush::increase_size(const int size) {
	set_size(this->size + size);
}
void Brush::decrease_size(const int size) {
	set_size(this->size - size);
}

void Brush::render(Terrain& terrain) const {
	gl->glDisable(GL_DEPTH_TEST);

	shader->use();

	// +3 for uv_offset so it can move sub terrain cell
	const int cells = std::ceil(((this->size * 2 + 1) * granularity + 3) / 4.f);

	gl->glUniformMatrix4fv(1, 1, GL_FALSE, &camera->projection_view[0][0]);
	gl->glUniform2f(2, position.x, position.y);
	gl->glUniform2f(3, uv_offset.x, uv_offset.y);

	gl->glBindTextureUnit(0, terrain.ground_corner_height);
	gl->glBindTextureUnit(1, brush_texture);

	gl->glEnableVertexAttribArray(0);
	gl->glBindBuffer(GL_ARRAY_BUFFER, shapes.vertex_buffer);
	gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes.index_buffer);
	gl->glDrawElementsInstanced(GL_TRIANGLES, shapes.quad_indices.size() * 3, GL_UNSIGNED_INT, nullptr, cells * cells);

	gl->glDisableVertexAttribArray(0);
	gl->glEnable(GL_DEPTH_TEST);
}