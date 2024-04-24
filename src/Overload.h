#pragma once

template<class... Ts>
struct Overload : Ts... { using Ts::operator()...; };
