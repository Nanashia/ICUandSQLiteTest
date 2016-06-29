#pragma once

template<class... Ts>
void print(Ts... args) {
	_print_internal(args...);
	std::cout << std::endl;
}

template<class T, class... Ts>
void _print_internal(T v, Ts... args) {
	std::cout << v;
	_print_internal(args...);
}

template<class T>
void _print_internal(T v) {
	std::cout << v;
}

std::ostream &operator<<(std::ostream &os, char* const &str) {
	return os << std::string(str);
}

