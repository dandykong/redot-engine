/**************************************************************************/
/*  audio_frame.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2024-present Redot Engine contributors                   */
/*                                          (see REDOT_AUTHORS.md)        */
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/math/vector2.h"
#include "core/typedefs.h"

static _FORCE_INLINE_ float undenormalize(float f) {
	union {
		uint32_t i;
		float f;
	} v;

	v.f = f;

	// original: return (v.i & 0x7f800000) == 0 ? 0.0f : f;
	// version from Tim Blechmann:
	return (v.i & 0x7f800000) < 0x08000000 ? 0.0f : f;
}

static const float AUDIO_PEAK_OFFSET = 0.0000000001f;
static const float AUDIO_MIN_PEAK_DB = -200.0f; // linear_to_db(AUDIO_PEAK_OFFSET)

struct AudioFrame {
	// Left and right samples.
	union {
		// NOLINTBEGIN(modernize-use-default-member-init)
		struct {
			float left;
			float right;
		};
#ifndef DISABLE_DEPRECATED
		struct {
			float l;
			float r;
		};
#endif
		float levels[2] = { 0.0 };
		// NOLINTEND(modernize-use-default-member-init)
	};

	_ALWAYS_INLINE_ const float &operator[](int p_idx) const {
		DEV_ASSERT((unsigned int)p_idx < 2);
		return levels[p_idx];
	}
	_ALWAYS_INLINE_ float &operator[](int p_idx) {
		DEV_ASSERT((unsigned int)p_idx < 2);
		return levels[p_idx];
	}

	constexpr AudioFrame operator+(const AudioFrame &p_frame) const { return AudioFrame(left + p_frame.left, right + p_frame.right); }
	constexpr AudioFrame operator-(const AudioFrame &p_frame) const { return AudioFrame(left - p_frame.left, right - p_frame.right); }
	constexpr AudioFrame operator*(const AudioFrame &p_frame) const { return AudioFrame(left * p_frame.left, right * p_frame.right); }
	constexpr AudioFrame operator/(const AudioFrame &p_frame) const { return AudioFrame(left / p_frame.left, right / p_frame.right); }

	constexpr AudioFrame operator+(float p_sample) const { return AudioFrame(left + p_sample, right + p_sample); }
	constexpr AudioFrame operator-(float p_sample) const { return AudioFrame(left - p_sample, right - p_sample); }
	constexpr AudioFrame operator*(float p_sample) const { return AudioFrame(left * p_sample, right * p_sample); }
	constexpr AudioFrame operator/(float p_sample) const { return AudioFrame(left / p_sample, right / p_sample); }

	constexpr void operator+=(const AudioFrame &p_frame) {
		left += p_frame.left;
		right += p_frame.right;
	}
	constexpr void operator-=(const AudioFrame &p_frame) {
		left -= p_frame.left;
		right -= p_frame.right;
	}
	constexpr void operator*=(const AudioFrame &p_frame) {
		left *= p_frame.left;
		right *= p_frame.right;
	}
	constexpr void operator/=(const AudioFrame &p_frame) {
		left /= p_frame.left;
		right /= p_frame.right;
	}

	constexpr void operator+=(float p_sample) {
		left += p_sample;
		right += p_sample;
	}
	constexpr void operator-=(float p_sample) {
		left -= p_sample;
		right -= p_sample;
	}
	constexpr void operator*=(float p_sample) {
		left *= p_sample;
		right *= p_sample;
	}
	constexpr void operator/=(float p_sample) {
		left /= p_sample;
		right /= p_sample;
	}

	_ALWAYS_INLINE_ void undenormalize() {
		left = ::undenormalize(left);
		right = ::undenormalize(right);
	}

	_FORCE_INLINE_ AudioFrame lerp(const AudioFrame &p_b, float p_t) const {
		AudioFrame res = *this;

		res.left += (p_t * (p_b.left - left));
		res.right += (p_t * (p_b.right - right));

		return res;
	}

	// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init)
	constexpr AudioFrame(float p_left, float p_right) :
			left(p_left), right(p_right) {}
	constexpr AudioFrame(const AudioFrame &p_frame) :
			left(p_frame.left), right(p_frame.right) {}
	// NOLINTEND(cppcoreguidelines-pro-type-member-init)

	constexpr void operator=(const AudioFrame &p_frame) {
		left = p_frame.left;
		right = p_frame.right;
	}

	constexpr operator Vector2() const {
		return Vector2(left, right);
	}

	// NOLINTBEGIN(cppcoreguidelines-pro-type-member-init)
	constexpr AudioFrame(const Vector2 &p_v2) :
			left(p_v2.x), right(p_v2.y) {}
	constexpr AudioFrame() :
			left(0), right(0) {}
	// NOLINTEND(cppcoreguidelines-pro-type-member-init)
};

constexpr AudioFrame operator*(float p_scalar, const AudioFrame &p_frame) {
	return AudioFrame(p_frame.left * p_scalar, p_frame.right * p_scalar);
}

constexpr AudioFrame operator*(int32_t p_scalar, const AudioFrame &p_frame) {
	return AudioFrame(p_frame.left * p_scalar, p_frame.right * p_scalar);
}

constexpr AudioFrame operator*(int64_t p_scalar, const AudioFrame &p_frame) {
	return AudioFrame(p_frame.left * p_scalar, p_frame.right * p_scalar);
}

template <>
struct is_zero_constructible<AudioFrame> : std::true_type {};
