#pragma once

#define MAX_CAPTURE_COUNT 4

struct Value;

using Box = Value*;

struct Fun {
	int capture_count;
	Box captures[MAX_CAPTURE_COUNT];
	int segment;
};

struct Value {
	enum class Tag { Int, Fun, Box };
	Tag tag;
	union {
		int as_int;
		Fun as_fun;
		Box as_box;
	};

	static Value Int(int x) {
		Value result;
		result.tag = Tag::Int;
		result.as_int = x;
		return result;
	}

	static Value Fun(int segment, int capture_count, Box* captures) {
		Value result;
		result.tag = Tag::Fun;
		result.as_fun.segment = segment;
		result.as_fun.capture_count = capture_count;
		for (int i = 0; i < capture_count; ++i) {
			result.as_fun.captures[i] = captures[i];
		}
		return result;
	}

	static Value Box(Box x) {
		Value result;
		result.tag = Tag::Box;
		result.as_box = x;
		return result;
	}

};
