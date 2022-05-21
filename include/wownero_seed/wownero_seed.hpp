/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <iostream>
#include <ctime>
#include "gf_poly.hpp"

class wownero_seed {
public:
	static const std::string erasure;
	static constexpr size_t size = 16;
	static constexpr size_t key_size = 32;
	using secret_key = std::array<uint8_t, key_size>;
	using secret_seed = std::array<uint8_t, size>;
	wownero_seed(const std::string& phrase, const std::string& coin);
	wownero_seed(std::time_t date_created, const std::string& coin);
	std::time_t date() const {
		return date_;
	}
    unsigned blockheight() const;
	const std::string& correction() const {
		return correction_;
	}
	const secret_key& key() const {
		return key_;
	}
	friend std::ostream& operator<<(std::ostream& os, const wownero_seed& seed);
private:
	secret_seed seed_;
	secret_key key_;
	std::time_t date_;
	unsigned reserved_;
	std::string correction_;
	gf_poly message_;
};

std::ostream& operator<<(std::ostream& os, const wownero_seed::secret_key& key);
