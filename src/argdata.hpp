// Copyright (c) 2017 Maurice Bos <m-ou.se@m-ou.se>.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#ifndef ARGDATA_HPP
#define ARGDATA_HPP

#include <time.h>

#include <chrono>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "argdata.h"

struct argdata_t {

	// TODO(ed): Remove this and use std::span<T> once available.
	template<typename T>
	using span = std::basic_string_view<T>;

	typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> timestamp;

	argdata_t() = delete;
	argdata_t(argdata_t const &) = delete;
	argdata_t &operator=(argdata_t const &) = delete;

	void operator delete (void *p) {
		argdata_free(static_cast<argdata_t *>(p));
	}

	static std::unique_ptr<argdata_t> create_from_buffer(span<unsigned char const> r, int (*convert_fd)(void *, size_t) = nullptr, void *convert_fd_arg = nullptr) {
		return std::unique_ptr<argdata_t>(argdata_from_buffer(r.data(), r.size(), convert_fd, convert_fd_arg));
	}
	static std::unique_ptr<argdata_t> create_binary(span<unsigned char const> r) {
		return std::unique_ptr<argdata_t>(argdata_create_binary(r.data(), r.size()));
	}
	static std::unique_ptr<argdata_t> create_fd(int v) {
		return std::unique_ptr<argdata_t>(argdata_create_fd(v));
	}
	static std::unique_ptr<argdata_t> create_float(double v) {
		return std::unique_ptr<argdata_t>(argdata_create_float(v));
	}
	template<typename T>
	static std::unique_ptr<argdata_t> create_int(T const &v) {
		return std::unique_ptr<argdata_t>(argdata_create_int(v));
	}
	static std::unique_ptr<argdata_t> create_int(int v) {
		return std::unique_ptr<argdata_t>(argdata_create_int_s(v));
	}
	static std::unique_ptr<argdata_t> create_str(std::string_view v) {
		return std::unique_ptr<argdata_t>(argdata_create_str(v.data(), v.size()));
	}
	static std::unique_ptr<argdata_t> create_timestamp(timestamp const & v) {
		std::chrono::nanoseconds d = v.time_since_epoch();
		timespec ts;
		ts.tv_sec = d.count() / 1000000000;
		ts.tv_nsec = d.count() % 1000000000;
		return std::unique_ptr<argdata_t>(argdata_create_timestamp(&ts));
	}

	static std::unique_ptr<argdata_t> create_map(span<argdata_t const *const> keys, span<argdata_t const *const> values) {
		return std::unique_ptr<argdata_t>(argdata_create_map(
			keys.data(),
			values.data(),
			keys.size() < values.size() ? keys.size() : values.size()
		));
	}

	static std::unique_ptr<argdata_t> create_seq(span<argdata_t const *const> values) {
		return std::unique_ptr<argdata_t>(argdata_create_seq(values.data(), values.size()));
	}

	static constexpr argdata_t const *false_() { return &argdata_false; }
	static constexpr argdata_t const *true_ () { return &argdata_true ; }
	static constexpr argdata_t const *null  () { return &argdata_null ; }

	static constexpr argdata_t const *bool_(bool v) { return v ? true_() : false_(); }

	std::optional<span<unsigned char const>> get_binary() const {
		void const *data;
		size_t size;
		if (argdata_get_binary(this, &data, &size)) return {};
		return span<unsigned char const>{static_cast<unsigned char const *>(data), size};
	}
	std::optional<bool> get_bool() const {
		bool r;
		if (argdata_get_bool(this, &r)) return {};
		return r;
	}
	std::optional<int> get_fd() const {
		int r;
		if (argdata_get_fd(this, &r)) return {};
		return r;
	}
	std::optional<double> get_float() const {
		double r;
		if (argdata_get_float(this, &r)) return {};
		return r;
	}
	template<typename T>
	std::optional<T> get_int() const {
		T r;
		if (argdata_get_int(this, &r)) return {};
		return r;
	}
	std::optional<std::string_view> get_str() const {
		char const *data;
		size_t size;
		if (argdata_get_str(this, &data, &size)) return {};
		return std::string_view(data, size);
	}
	std::optional<timestamp> get_timestamp() const {
		timespec r;
		if (argdata_get_timestamp(this, &r)) return {};
		return timestamp(std::chrono::seconds(r.tv_sec) + std::chrono::nanoseconds(r.tv_nsec));
	}

	// Same as above, but return a default value (empty/zero/etc.) instead of nullopt.
	span<unsigned char const> as_binary   () const { return get_binary   ().value_or(           nullptr); }
	bool                      as_bool     () const { return get_bool     ().value_or(             false); }
	int                       as_fd       () const { return get_fd       ().value_or(                -1); }
	double                    as_float    () const { return get_float    ().value_or(               0.0); }
	template<typename T> T    as_int      () const { return get_int<T>   ().value_or(                 0); }
	std::string_view          as_str      () const { return get_str      ().value_or(std::string_view{}); }
	timestamp                 as_timestamp() const { return get_timestamp().value_or(       timestamp{}); }

	class map;
	class seq;

	class map_iterator {
	public:
		using value_type = std::pair<argdata_t const *, argdata_t const *>;
		using pointer = value_type const *;
		using reference = value_type const &;
		using iterator_category = std::forward_iterator_tag;
	private:
		value_type value_;
		argdata_map_iterator_t it_;
		friend map;
	public:
		map_iterator() { it_.index = ARGDATA_ITERATOR_END; }
		reference operator*() {
			argdata_map_get(&it_, &value_.first, &value_.second);
			return value_;
		}
		pointer operator->() {
			return &this->operator*();
		}
		map_iterator &operator++() {
			argdata_map_next(&it_);
			return *this;
		}
		map_iterator operator++(int) {
			map_iterator copy = *this;
			++*this;
			return copy;
		}
		size_t index() const {
			return it_.index;
		}
		bool error() const {
			return it_.index == ARGDATA_ITERATOR_INVALID;
		}
		friend bool operator==(map_iterator const &a, map_iterator const &b) {
			return a.it_.index == b.it_.index;
		}
		friend bool operator!=(map_iterator const &a, map_iterator const &b) {
			return a.it_.index != b.it_.index;
		}
		friend bool operator<(map_iterator const &a, map_iterator const &b) {
			return a.it_.index < b.it_.index;
		}
		friend bool operator>(map_iterator const &a, map_iterator const &b) {
			return a.it_.index > b.it_.index;
		}
		friend bool operator<=(map_iterator const &a, map_iterator const &b) {
			return a.it_.index <= b.it_.index;
		}
		friend bool operator>=(map_iterator const &a, map_iterator const &b) {
			return a.it_.index >= b.it_.index;
		}
	};

	class seq_iterator {
	public:
		using value_type = argdata_t const *;
		using pointer = value_type const *;
		using reference = value_type const &;
		using iterator_category = std::forward_iterator_tag;
	private:
		value_type value_ = nullptr;
		argdata_seq_iterator_t it_;
		friend seq;
	public:
		seq_iterator() { it_.index = ARGDATA_ITERATOR_END; }
		reference operator*() {
			argdata_seq_get(&it_, &value_);
			return value_;
		}
		pointer operator->() {
			return &this->operator*();
		}
		seq_iterator &operator++() {
			argdata_seq_next(&it_);
			return *this;
		}
		seq_iterator operator++(int) {
			seq_iterator copy = *this;
			++*this;
			return copy;
		}
		size_t index() const {
			return it_.index;
		}
		bool error() const {
			return it_.index == ARGDATA_ITERATOR_INVALID;
		}
		friend bool operator==(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index == b.it_.index;
		}
		friend bool operator!=(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index != b.it_.index;
		}
		friend bool operator<(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index < b.it_.index;
		}
		friend bool operator>(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index > b.it_.index;
		}
		friend bool operator<=(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index <= b.it_.index;
		}
		friend bool operator>=(seq_iterator const &a, seq_iterator const &b) {
			return a.it_.index >= b.it_.index;
		}
	};

	class map {
	private:
		argdata_map_iterator_t start_it_;
		friend argdata_t;
	public:
		map_iterator begin() const {
			map_iterator i;
			i.it_ = start_it_;
			return i;
		}
		map_iterator end() const { return {}; }
	};

	class seq {
	private:
		argdata_seq_iterator_t start_it_;
		friend argdata_t;
	public:
		seq_iterator begin() const {
			seq_iterator i;
			i.it_ = start_it_;
			return i;
		}
		seq_iterator end() const { return {}; }
	};

	std::optional<map> get_map() const {
		map r;
		argdata_map_iterate(this, &r.start_it_);
		if (r.start_it_.index == ARGDATA_ITERATOR_INVALID) return {};
		return r;
	}
	std::optional<seq> get_seq() const {
		seq r;
		argdata_seq_iterate(this, &r.start_it_);
		if (r.start_it_.index == ARGDATA_ITERATOR_INVALID) return {};
		return r;
	}

	map as_map() const {
		map r;
		argdata_map_iterate(this, &r.start_it_);
		return r;
	}
	seq as_seq() const {
		seq r;
		argdata_seq_iterate(this, &r.start_it_);
		return r;
	}

	size_t serialized_length(size_t *n_fds = nullptr) const {
		size_t r;
		argdata_serialized_length(this, &r, n_fds);
		return r;
	}

	void serialize(std::vector<unsigned char> &buffer) const {
		buffer.resize(serialized_length());
		argdata_serialize(this, buffer.data(), nullptr);
	}

	void serialize(std::vector<unsigned char> &buffer, std::vector<int> &fds) const {
		size_t n_fds;
		buffer.resize(serialized_length(&n_fds));
		fds.resize(n_fds);
		n_fds = argdata_serialize(this, buffer.data(), fds.data());
		fds.resize(n_fds);
	}

	std::vector<unsigned char> serialize(std::vector<int> *fds = nullptr) const {
		std::vector<unsigned char> buffer;
		if (fds) serialize(buffer, *fds);
		else serialize(buffer);
		return buffer;
	}

};

struct argdata_reader_t {

	argdata_reader_t() = delete;
	argdata_reader_t(argdata_reader_t const &) = delete;
	argdata_reader_t &operator=(argdata_reader_t const &) = delete;

	void operator delete (void *p) {
		argdata_reader_free(static_cast<argdata_reader_t *>(p));
	}

	static std::unique_ptr<argdata_reader_t> create(size_t max_data_len, size_t max_fds_len) {
		return std::unique_ptr<argdata_reader_t>(argdata_reader_create(max_data_len, max_fds_len));
	}

	const argdata_t *get() {
		return argdata_reader_get(this);
	}
	int pull(int fd) {
		return argdata_reader_pull(this, fd);
	}
	void release_fd(int fd) {
		argdata_reader_release_fd(this, fd);
	}

};

struct argdata_writer_t {

	argdata_writer_t() = delete;
	argdata_writer_t(argdata_writer_t const &) = delete;
	argdata_writer_t &operator=(argdata_writer_t const &) = delete;

	void operator delete (void *p) {
		argdata_writer_free(static_cast<argdata_writer_t *>(p));
	}

	static std::unique_ptr<argdata_writer_t> create() {
		return std::unique_ptr<argdata_writer_t>(argdata_writer_create());
	}

	int push(int fd) {
		return argdata_writer_push(this, fd);
	}
	void set(const argdata_t *ad) {
		argdata_writer_set(this, ad);
	}

};

#endif
