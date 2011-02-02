#ifndef MAIN_H
#define MAIN_H
#include <iostream>
#include <string>

#include <sys/timeb.h>
#include "node.hpp"
#include "defs.hpp"
#include "data.hpp"
#include "search.hpp"

int main(int argc, char *argv[]);
int parse_move(std::vector<gen_t>& workq, const char *s);
char *move_str(move_bytes m);
void print_board(node_t& board, std::ostream& out);
int print_result(std::vector<gen_t>& workq, node_t& board);
void start_benchmark(std::string filename, int ply_level, int num_runs);
int get_ms();
std::string get_log_name();

#endif
