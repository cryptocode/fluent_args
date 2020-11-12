#pragma once

#define fluent_arg(T, N) T _##N; [[nodiscard]] action& N(T val) {_##N = val; return *this; }
