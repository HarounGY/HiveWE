#include "stdafx.h"

void Camera::reset() {
	position.z = 6;
	horizontal_angle = 0.0;
	vertical_angle = -0.977;
	update(0);
}

void FPSCamera::update(const double delta) {
	float speed = 5;
	if (input_handler.key_pressed(Qt::Key_Shift)) {
		speed = 20;
	}

	if (input_handler.key_pressed(Qt::Key_W)) {
		position += direction * speed * float(delta);
	} else if (input_handler.key_pressed(Qt::Key_S)) {
		position -= direction * speed * float(delta);
	}

	const glm::vec3 displacement = glm::normalize(glm::cross(direction, up)) * speed * float(delta);
	if (input_handler.key_pressed(Qt::Key_A)) {
		position -= displacement;
	} else if (input_handler.key_pressed(Qt::Key_D)) {
		position += displacement;
	}

	if (input_handler.key_pressed(Qt::Key_Space)) {
		position.z += 1 * speed * float(delta);
	} else if (input_handler.key_pressed(Qt::Key_Control)) {
		position.z -= 1 * speed * float(delta);
	}
	
	direction = glm::vec3(
		std::cos(vertical_angle) * std::sin(horizontal_angle),
		std::cos(vertical_angle) * std::cos(horizontal_angle),
		std::sin(vertical_angle)
	);

	direction = glm::normalize(direction);

	projection = glm::perspective(fov, aspect_ratio, draw_distance_close, draw_distance);
	view = glm::lookAt(position, position + direction, up);
	projection_view = projection * view;
}

void FPSCamera::mouse_move_event(QMouseEvent* event) {
	const int diffx = input_handler.mouse.x() - input_handler.previous_mouse.x();
	const int diffy = input_handler.mouse.y() - input_handler.previous_mouse.y();

	horizontal_angle += diffx * 0.012;
	vertical_angle += -diffy * 0.012;
	vertical_angle = std::max(-glm::pi<double>() / 2 + 0.001, std::min(vertical_angle, glm::pi<double>() / 2 - 0.001));

	update(0);

	input_handler.previous_mouse = event->globalPos();
}

void FPSCamera::mouse_scroll_event(QWheelEvent* event) {
}


void TPSCamera::update(double delta) {
	direction = glm::vec3(
		std::cos(vertical_angle) * std::sin(horizontal_angle),
		std::cos(vertical_angle) * std::cos(horizontal_angle),
		std::sin(vertical_angle)
	);

	direction = glm::normalize(direction);
	// Calculate axis directions for camera as referential point:
	// Z axis is simply the direction we are facing
	// X axis is then the cross product between the "fake" up and Z
	X = glm::cross(direction, up);
	X = glm::normalize(X);
	// Y Axis is cross product between X and Z, e.g is the real up
	Y = glm::cross(X, direction);
	Y = glm::normalize(Y);

	// The vector that is perpendicular to the up vector, thus points forward
	forward = glm::cross(X, up);

	projection = glm::perspective(fov, aspect_ratio, draw_distance_close, draw_distance);
	view = glm::lookAt(position, position + direction, up);
	projection_view = projection * view;
}

void TPSCamera::mouse_move_event(QMouseEvent* event) {
	const int diffx = input_handler.mouse.x() - input_handler.previous_mouse.x();
	const int diffy = input_handler.mouse.y() - input_handler.previous_mouse.y();
	if (event->buttons() == Qt::RightButton) {
		position += X * (-diffx * 0.025f);
		position += forward * (-diffy * 0.025f);

		update(0);
	}

	if (rolling) {
		horizontal_angle += -diffx * 0.0025;
		vertical_angle += diffy * 0.0025;
		vertical_angle = std::max(-glm::pi<double>() / 2 + 0.001, std::min(vertical_angle, glm::pi<double>() / 2 - 0.001));
		update(0);
	}

	input_handler.previous_mouse = event->globalPos();
}

void TPSCamera::mouse_scroll_event(QWheelEvent* event) {
	position += direction * (event->angleDelta().y() * 0.025f);
	update(0);
}

void TPSCamera::mouse_press_event(QMouseEvent* event) {
	switch (event->button()) {
		case Qt::MiddleButton:
			rolling = true;
			break;
	}
}

void TPSCamera::mouse_release_event(QMouseEvent* event) {
	switch (event->button()) {
		case Qt::MiddleButton:
			rolling = false;
			break;
	}
}

//Checks if a point is within the defined camera frustum.
//The point is expected to be in the same coordinate system,
//i.e divide wc3 object positions by 128.0 before passing.
bool TPSCamera::is_visible(glm::vec3 && point) const {
	// Calculate the point position as the camera as referential point
	auto v = point - position;
	// Project the vector on Z-axis
	auto vZ = dot(v, -direction) * -direction;
	// Calculate length to get how far away the point is in z-axis
	auto z = glm::length(vZ);
	// Check if point is within frustum in z-axis (if its too far or too close)
	if (z > draw_distance || z < draw_distance_close) {
		return false;
	}

	// Some quick trigonometry to calculate height of at point distance
	// half_height / v.z = tan(fov / 2)
	auto fov_rad = (glm::pi<float>() / 180.f) * static_cast<float>(fov); // Need radians
	const float height = z * tan_height;
	// Calculate how far away the point is in Y-axis
	auto vY = dot(v, Y) * Y;
	auto y = glm::length(vY);

	if (y > height || y + height < 0) {
		return false;
	}

	// For x-axis similarly but width instead
	// use aspect ratio to get width
	auto width = height * aspect_ratio;
	// Again how far away the point is in X-axis
	auto vX = dot(v, X) * X;
	auto x = glm::length(vX);
	return x > width || x + width < 0;
}

Camera* camera;