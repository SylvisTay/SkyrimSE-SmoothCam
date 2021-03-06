#include "camera_states/thirdperson.h"
#include "camera.h"

/*
 * The base thirdperson camera logic - used when no weapon is drawn.
 */

Camera::State::ThirdpersonState::ThirdpersonState(Camera::SmoothCamera* camera) noexcept : BaseCameraState(camera) {

}

void Camera::State::ThirdpersonState::OnBegin(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {

}

void Camera::State::ThirdpersonState::OnEnd(const PlayerCharacter* player, const CorrectedPlayerCamera* camera) {

}

void Camera::State::ThirdpersonState::Update(PlayerCharacter* player, const CorrectedPlayerCamera* camera) {
	// Get our computed local-space xyz offset.
	const auto cameraLocal = GetCameraLocalPosition(player, camera);
	// Get the base world position for the camera which we will offset with the local-space values.
	const auto worldTarget = GetCameraWorldPosition(player, camera);
	// Transform the camera offsets based on the computed view matrix
	const auto transformedLocalPos = GetTransformedCameraLocalPosition(player, camera);
	// Define the starting point for our raycast
	const auto start = worldTarget + glm::vec3(0.0f, 0.0f, cameraLocal.z);

	glm::vec3 localPos;
	glm::vec3 preFinalPos;
	if (GetConfig()->separateLocalInterp) {
		// Handle separate local-space interpolation

		// Interpolate the local position (rotation and translation offsets)
		localPos = UpdateInterpolatedLocalPosition(player, transformedLocalPos);
		// And the world target
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(player, worldTarget, glm::length(GetLastWorldPosition() - worldTarget));
		// Compute offset clamping if enabled
		const auto clampedWorldPos = ComputeOffsetClamping(player, start, lerpedWorldPos);
		StoreLastWorldPosition(clampedWorldPos);

		// Construct the final position
		preFinalPos = clampedWorldPos + localPos;
	} else {
		// Combined case

		// Add the final local space transformation to the player postion
		const auto targetWorldPos = worldTarget + transformedLocalPos;
		// Now lerp it based on camera distance to player position
		const auto lerpedWorldPos = UpdateInterpolatedWorldPosition(player, targetWorldPos, glm::length(targetWorldPos - worldTarget));
		// Compute offset clamping if enabled
		preFinalPos = ComputeOffsetClamping(player, camera, transformedLocalPos, worldTarget, lerpedWorldPos);
		StoreLastWorldPosition(preFinalPos);
		localPos = lerpedWorldPos - worldTarget;
	}

	// Cast our ray and update the camera position
	const auto finalPos = ComputeRaycast(start, preFinalPos);
	// Set the position
	SetCameraPosition(finalPos, camera);

	// Feed our local position offsets to the game camera state for correct crosshair alignment
	ApplyLocalSpaceGameOffsets(localPos, player, camera);

	// Update crosshair visibility
	UpdateCrosshair(player, camera);
}