/**
 * ARC+ Internal v2 — Intercept Solvers
 *
 * Pure math functions for solving projectile intercept problems.
 * No game dependencies — uses its own Vec3 type.
 * All functions are inline for maximum optimization.
 */

#pragma once

#include <cmath>

namespace prediction::math
{

// ============================================================================
// Vec3 — prediction-local vector type (no SDK dependency)
// ============================================================================

struct Vec3
{
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3 operator+(const Vec3& o) const
	{
		return {x + o.x, y + o.y, z + o.z};
	}

	Vec3 operator-(const Vec3& o) const
	{
		return {x - o.x, y - o.y, z - o.z};
	}

	Vec3 operator*(float s) const
	{
		return {x * s, y * s, z * s};
	}

	Vec3 operator/(float s) const
	{
		float inv = 1.0f / s;
		return {x * inv, y * inv, z * inv};
	}

	Vec3& operator+=(const Vec3& o)
	{
		x += o.x;
		y += o.y;
		z += o.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& o)
	{
		x -= o.x;
		y -= o.y;
		z -= o.z;
		return *this;
	}

	float dot(const Vec3& o) const
	{
		return x * o.x + y * o.y + z * o.z;
	}

	float length_sq() const
	{
		return x * x + y * y + z * z;
	}

	float length() const
	{
		return std::sqrt(length_sq());
	}

	float distance(const Vec3& o) const
	{
		return (*this - o).length();
	}

	Vec3 normalized() const
	{
		float len = length();
		if (len < 1e-6f)
			return {0, 0, 0};
		return *this / len;
	}
};

// ============================================================================
// Result types
// ============================================================================

struct InterceptResult
{
	bool valid; // Solution found?
	Vec3 aim_point; // Where to aim (world coords)
	float time; // Time to intercept (seconds)
	float lead_distance; // How far aim shifted from current pos
	float confidence; // 0.0-1.0 quality score
};

struct CPAResult
{
	float time; // Time of closest approach
	float distance; // Minimum distance at CPA
	Vec3 point_a; // Position of A at CPA
	Vec3 point_b; // Position of B at CPA
};

// ============================================================================
// Analytical linear intercept (no gravity)
// ============================================================================

// Solves: |target_pos + target_vel*t - source| = speed * t
// Expands to quadratic: (|V|^2 - speed^2)*t^2 + 2*(D.V)*t + |D|^2 = 0
// Returns lowest positive t.
inline InterceptResult solve_linear(
	const Vec3& source,
	const Vec3& target_pos,
	const Vec3& target_vel,
	float projectile_speed)
{
	InterceptResult result{};
	result.valid = false;

	if (projectile_speed <= 0.0f)
		return result;

	Vec3 D = target_pos - source;
	const Vec3& V = target_vel;

	float a = V.dot(V) - projectile_speed * projectile_speed;
	float b = 2.0f * D.dot(V);
	float c = D.dot(D);

	float t = -1.0f;

	// Degenerate case: a ~= 0 (target speed ~= projectile speed)
	if (std::abs(a) < 1e-6f)
	{
		if (std::abs(b) < 1e-6f)
			return result; // No solution
		t = -c / b;
	}
	else
	{
		float disc = b * b - 4.0f * a * c;
		if (disc < 0.0f)
			return result; // No real solution

		float sqrt_disc = std::sqrt(disc);
		float inv_2a = 1.0f / (2.0f * a);

		float t1 = (-b - sqrt_disc) * inv_2a;
		float t2 = (-b + sqrt_disc) * inv_2a;

		// Pick smallest positive root
		if (t1 > 0.001f && t2 > 0.001f)
			t = (t1 < t2) ? t1 : t2;
		else if (t1 > 0.001f)
			t = t1;
		else if (t2 > 0.001f)
			t = t2;
		else
			return result;
	}

	result.valid = true;
	result.aim_point = target_pos + V * t;
	result.time = t;
	result.lead_distance = (result.aim_point - target_pos).length();
	result.confidence = 1.0f;

	return result;
}

// ============================================================================
// Iterative ballistic intercept (projectile has gravity)
// ============================================================================

// Source 2 projectiles arc on their own — the engine applies gravity_scale
// to the projectile's trajectory. We do NOT aim higher to compensate for
// drop; instead we aim directly at the predicted target position. Gravity
// only affects our flight time estimate (arcing path ≠ straight line).
// gravity_accel = downward acceleration (positive = down, e.g. 160.0 for 0.2 scale)
inline InterceptResult solve_ballistic(
	const Vec3& source,
	const Vec3& target_pos,
	const Vec3& target_vel,
	float projectile_speed,
	float gravity_accel,
	int max_iterations = 8)
{
	InterceptResult result{};
	result.valid = false;

	if (projectile_speed <= 0.0f)
		return result;

	// No gravity? Use analytical solver.
	if (gravity_accel < 0.001f)
	{
		return solve_linear(source, target_pos, target_vel, projectile_speed);
	}

	// Seed with linear estimate
	Vec3 D = target_pos - source;
	float dist = D.length();
	if (dist < 1.0f)
		return result;

	float t = dist / projectile_speed;

	for (int i = 0; i < max_iterations; i++)
	{
		if (t <= 0.0f || t > 10.0f)
			return result; // Unrealistic

		// Predict target position at time t — aim directly there.
		// The engine handles the projectile arc; we just need to point
		// at where the target will be when the projectile arrives.
		Vec3 aim = target_pos + target_vel * t;

		// Recalculate distance from source to aim point
		float new_dist = (aim - source).length();
		if (new_dist < 1.0f)
			break;

		// Gravity-adjusted travel time: the arcing flight path is longer
		// than a straight line. When shooting upward, gravity slows the
		// projectile (longer travel). When shooting down, gravity speeds
		// it up (shorter travel).
		Vec3 dir = (aim - source) / new_dist;
		float gravity_along_path = -gravity_accel * dir.z; // positive when shooting down
		float avg_speed = projectile_speed + 0.5f * gravity_along_path * t;
		if (avg_speed < projectile_speed * 0.25f)
			avg_speed = projectile_speed * 0.25f;

		float new_t = new_dist / avg_speed;

		// Check convergence
		if (std::abs(new_t - t) < 0.001f)
		{
			t = new_t;
			break;
		}
		t = new_t;
	}

	if (t <= 0.0f || t > 10.0f)
		return result;

	// Final aim point — directly at predicted target, no Z offset
	Vec3 aim = target_pos + target_vel * t;

	result.valid = true;
	result.aim_point = aim;
	result.time = t;
	result.lead_distance = (aim - target_pos).length();
	result.confidence = 1.0f;

	return result;
}

// ============================================================================
// Ballistic with target also affected by gravity (airborne targets)
// ============================================================================

inline InterceptResult solve_ballistic_both(
	const Vec3& source,
	const Vec3& target_pos,
	const Vec3& target_vel,
	float target_gravity,
	float projectile_speed,
	float projectile_gravity,
	int max_iterations = 8)
{
	InterceptResult result{};
	result.valid = false;

	if (projectile_speed <= 0.0f)
		return result;

	Vec3 D = target_pos - source;
	float dist = D.length();
	if (dist < 1.0f)
		return result;

	float t = dist / projectile_speed;

	for (int i = 0; i < max_iterations; i++)
	{
		if (t <= 0.0f || t > 10.0f)
			return result;

		// Predict target position with gravity (airborne target falls down)
		Vec3 aim = target_pos + target_vel * t;
		aim.z -= 0.5f * target_gravity * t * t;

		// No projectile Z offset — engine handles the arc.
		// We aim directly at predicted target position.

		float new_dist = (aim - source).length();
		if (new_dist < 1.0f)
			break;

		// Gravity-adjusted travel time (arcing path ≠ straight line)
		Vec3 dir = (aim - source) / new_dist;
		float gravity_along_path = -projectile_gravity * dir.z;
		float avg_speed = projectile_speed + 0.5f * gravity_along_path * t;
		if (avg_speed < projectile_speed * 0.25f)
			avg_speed = projectile_speed * 0.25f;

		float new_t = new_dist / avg_speed;

		if (std::abs(new_t - t) < 0.001f)
		{
			t = new_t;
			break;
		}
		t = new_t;
	}

	if (t <= 0.0f || t > 10.0f)
		return result;

	// Final aim: predicted target with gravity drop, no projectile Z offset
	Vec3 aim = target_pos + target_vel * t;
	aim.z -= 0.5f * target_gravity * t * t;

	result.valid = true;
	result.aim_point = aim;
	result.time = t;
	result.lead_distance = (aim - target_pos).length();
	result.confidence = 1.0f;

	return result;
}

// ============================================================================
// Closest point of approach
// ============================================================================

// When will two linearly-moving objects be closest?
inline CPAResult closest_approach(
	const Vec3& pos_a, const Vec3& vel_a,
	const Vec3& pos_b, const Vec3& vel_b)
{
	CPAResult result{};

	Vec3 dv = vel_a - vel_b;
	Vec3 dp = pos_a - pos_b;

	float dv2 = dv.dot(dv);

	if (dv2 < 1e-6f)
	{
		// Parallel motion or both stationary
		result.time = 0.0f;
	}
	else
	{
		result.time = -dp.dot(dv) / dv2;
		if (result.time < 0.0f)
			result.time = 0.0f; // Already past
	}

	result.point_a = pos_a + vel_a * result.time;
	result.point_b = pos_b + vel_b * result.time;
	result.distance = result.point_a.distance(result.point_b);

	return result;
}

// ============================================================================
// Sphere intercept
// ============================================================================

// Time when a moving sphere (A) first touches another moving sphere (B).
// Returns negative if no collision.
inline float sphere_intercept_time(
	const Vec3& pos_a, const Vec3& vel_a, float radius_a,
	const Vec3& pos_b, const Vec3& vel_b, float radius_b)
{
	Vec3 dp = pos_b - pos_a;
	Vec3 dv = vel_b - vel_a;
	float r = radius_a + radius_b;

	float a = dv.dot(dv);
	float b = 2.0f * dp.dot(dv);
	float c = dp.dot(dp) - r * r;

	// Already overlapping
	if (c < 0.0f)
		return 0.0f;

	if (std::abs(a) < 1e-6f)
		return -1.0f; // Not approaching

	float disc = b * b - 4.0f * a * c;
	if (disc < 0.0f)
		return -1.0f; // Miss

	float sqrt_disc = std::sqrt(disc);
	float inv_2a = 1.0f / (2.0f * a);

	float t1 = (-b - sqrt_disc) * inv_2a;
	float t2 = (-b + sqrt_disc) * inv_2a;

	if (t1 > 0.0f)
		return t1;
	if (t2 > 0.0f)
		return t2;
	return -1.0f;
}

}