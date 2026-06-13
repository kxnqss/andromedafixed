/**
 * ARC+ Internal v2 — Trajectory Simulation & Extrapolation
 *
 * Pure math helpers for position extrapolation and projectile paths.
 * No game dependencies. Header-only for inlining.
 */

#pragma once

#include "intercept.hpp" // Vec3

namespace prediction::math
{

// ============================================================================
// Position extrapolation
// ============================================================================

inline Vec3 extrapolate_linear(const Vec3& pos, const Vec3& vel, float dt)
{
	return pos + vel * dt;
}

inline Vec3 extrapolate_accel(const Vec3& pos, const Vec3& vel, const Vec3& accel, float dt)
{
	// p = p0 + v*t + 0.5*a*t^2
	return pos + vel * dt + accel * (0.5f * dt * dt);
}

inline Vec3 extrapolate_gravity(const Vec3& pos, const Vec3& vel, float gravity, float dt)
{
	Vec3 result = pos + vel * dt;
	result.z -= 0.5f * gravity * dt * dt;
	return result;
}

// ============================================================================
// Projectile position at time T
// ============================================================================

inline Vec3 projectile_position(
	const Vec3& origin,
	const Vec3& direction,
	float speed,
	float gravity,
	float dt)
{
	Vec3 result = origin + direction * (speed * dt);
	result.z -= 0.5f * gravity * dt * dt;
	return result;
}

// ============================================================================
// Travel time calculations
// ============================================================================

inline float travel_time_linear(float distance, float speed)
{
	if (speed <= 0.0f)
		return -1.0f;
	return distance / speed;
}

// Approximate travel time for a ballistic projectile over a given horizontal
// distance with a height difference. Uses iterative refinement.
inline float travel_time_ballistic(float distance, float speed, float gravity, float height_diff)
{
	if (speed <= 0.0f)
		return -1.0f;
	if (gravity < 0.001f)
		return distance / speed;

	// Seed with linear estimate
	float t = distance / speed;

	// Refine: account for the longer path due to arc
	for (int i = 0; i < 4; i++)
	{
		float drop = 0.5f * gravity * t * t;
		float actual_dist = std::sqrt(distance * distance + (height_diff + drop) * (height_diff + drop));
		t = actual_dist / speed;
	}

	return t;
}

// How long until an entity at pos+vel reaches target_pos moving at max_speed?
inline float time_to_reach(const Vec3& pos, const Vec3& vel, const Vec3& target, float max_speed)
{
	if (max_speed <= 0.0f)
		return -1.0f;

	float dist = pos.distance(target);
	float closing_speed = vel.dot((target - pos).normalized());

	// If already moving toward target, effective speed is higher
	float effective_speed = closing_speed > 0.0f
		                        ? (closing_speed < max_speed ? max_speed : closing_speed)
		                        : max_speed;

	return dist / effective_speed;
}

// ============================================================================
// Angle & range constraints
// ============================================================================

// Check if aim pitch would exceed [-89, 89] degrees
inline bool is_angle_reachable(const Vec3& source, const Vec3& aim_point)
{
	Vec3 delta = aim_point - source;
	float horiz = std::sqrt(delta.x * delta.x + delta.y * delta.y);
	if (horiz < 0.001f)
		return true; // Directly above/below — reachable

	float pitch_rad = std::atan2(-delta.z, horiz);
	float pitch_deg = pitch_rad * (180.0f / 3.14159265f);

	return pitch_deg >= -89.0f && pitch_deg <= 89.0f;
}

inline bool is_in_range(const Vec3& source, const Vec3& aim_point, float max_range)
{
	if (max_range <= 0.0f)
		return true; // Unlimited
	return source.distance(aim_point) <= max_range;
}

}