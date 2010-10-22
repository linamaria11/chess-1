/*
 *  MAIN.CPP
 */

#include "main.hpp"
#include "optlist.hpp"
#include <signal.h>
#include <sys/time.h>
#include <math.h>

struct timeval startt;
double sum_exec_times2 = 0;
double sum_exec_times = 0;
int count_exec_times;

int parseArgs(int, char**);

/* main() is basically an infinite loop that either calls
   think() when it's the computer's turn to move or prompts
   the user for a command (and deciphers it). */

int auto_move = 0;
int computer_side;

int chx_main(int argc, char **argv)
{
    int arguments = parseArgs(argc,argv);

    std::string s;
    int m;

    // If there were no command line arguments, display message
    if (!arguments) {
        std::cout << std::endl;
        std::cout << "Chess (CHX) Serial Minimax Version" << std::endl;
        std::cout << "Phillip LeBlanc - CCT" << std::endl;
        std::cout << std::endl;
        std::cout << "\"help\" displays a list of commands." << std::endl;
        std::cout << std::endl;
        computer_side = EMPTY;
    }
    else  // Else we start making moves
    {
        auto_move = 1;
    }
    node_t board;  // The board state is represented in the node_t struct

    std::vector<gen_t> workq;  // workq is a standard vector which contains
                               // all possible psuedo-legal moves for the
                               // current board position
    gen(workq, board);  // gen() takes the current board position and
                        // puts all of the psuedo-legal moves for the
                        // position inside of the workq vector.

    for (;;) {
        if (board.side == computer_side) {  /* computer's turn */

            /* think about the move and make it */
            think(board);
            if (move_to_make.u == 0) {
                std::cout << "(no legal moves)" << std::endl;
                computer_side = EMPTY;
                continue;
            }
            if (output)
                std::cout << "Computer's move: " << move_str(move_to_make.b)
                          << std::endl;
            makemove(board, move_to_make.b); // Make the move for our master
                                             // board
            board.ply = 0; // Reset the board ply to 0

            workq.clear(); // Clear the work queue in preparation for next move
            gen(workq, board); // Populate the work queue with the moves for
                               // the next board position

            if (output)
                print_board(board, stdout);
            if (auto_move) {
                auto_move = print_result(workq, board);
	    	if(!auto_move) {
	    	    struct timeval endt;
	    	    gettimeofday(&endt,0);
	    	    double st = startt.tv_sec+1.0e-6*startt.tv_usec;
	    	    double et = endt.tv_sec+1.0e-6*endt.tv_usec;
		    sum_exec_times += (et-st);
		    sum_exec_times2 += (et-st)*(et-st);
		    count_exec_times ++;
		    std::cout << " count  = " << count_exec_times << std::endl;
      		    std::cout << " time = " << (et-st) << std::endl;
		    double avg = sum_exec_times / count_exec_times;
		    std::cout << " avg  = " << avg << std::endl;
		    if(count_exec_times > 1) {
			double f = (1.0*count_exec_times)/(count_exec_times-1);
		    	double avg2 = sum_exec_times2 / (count_exec_times-1);
			double stdv = sqrt(avg2-f*avg*avg);
		    	std::cout << " stdv  = " << stdv << std::endl;
		    }
		}
            } else
                print_result(workq, board);

            move_to_make.u = 0; // Reset the move to make

            continue;
        }

        if (auto_move)
        {
            computer_side = board.side;
            continue;
        }
        if (!auto_move && arguments)
        {
            break;
        }

        /* get user input */
        std::cout << "chx> ";
        std::cin >> s;
        if (s.empty())
            return 0;
        if (s == "go") {
            computer_side = board.side;
            auto_move = 0;
            continue;
        }
        if (s == "auto") {
            computer_side = board.side;
            auto_move = 1;
            auto_move = print_result(workq, board);
	    std::cout << "starting timer" << std::endl;
	    gettimeofday(&startt,0);
            continue;
        }
        if (s == "new") {
            computer_side = EMPTY;
            board.clear();
            workq.clear();
            gen(workq, board);
            continue;
        }
        if (s == "wd") {
            std::cin >> depth[LIGHT];
            if (depth[LIGHT] <= 0)
            {
                std::cerr << "Illegal depth given, setting depth to 1." 
                          << std::endl;
                depth[LIGHT] = 1;
            }
            continue;
        }
        if (s == "bd") {
            std::cin >> depth[DARK];
            if (depth[DARK] <= 0)
            {
                std::cerr << "Illegal depth given, setting depth to 1." 
                          << std::endl;
                depth[DARK] = 1;
            }
            continue;
        }
        if (s == "d") {
            print_board(board, stdout);
            continue;
        }
        if (s == "o") {
            output ^= 1;
            if (output == 1)
                std::cout << "Output is now on" << std::endl;
            else
                std::cout << "Output is now off" << std::endl;
            continue;
        }
        if ((s == "exit")||(s == "quit")) {
            std::cout << "Thanks for using CHX!" << std::endl;
            break;
        }
        if (s == "help") {
            std::cout << "go - computer makes a move" << std::endl;
            std::cout << "auto - computer will continue to make moves until game is over" << std::endl;
            std::cout << "new - starts a new game" << std::endl;
            std::cout << "wd n - sets white search depth to n (default 3)" << std::endl;
            std::cout << "bd n - sets black search depth to n (default 3)" << std::endl;
            std::cout << "d - display the board" << std::endl;
            std::cout << "o - toggles engine output on or off (default on)" << std::endl;
            std::cout << "exit - exit the program" << std::endl;
            std::cout << std::endl;
            continue;
        }

        std::cout << "Illegal command." << std::endl;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int retcode = chx_main(argc, argv);
    return retcode;
}


/* parse the move s (in coordinate notation) and return the move's
   int value, or -1 if the move is illegal */

int parse_move(std::vector<gen_t>& workq, const char *s)
{
    int from, to, i;

    /* make sure the string looks like a move */
    if (s[0] < 'a' || s[0] > 'h' ||
            s[1] < '0' || s[1] > '9' ||
            s[2] < 'a' || s[2] > 'h' ||
            s[3] < '0' || s[3] > '9')
        return -1;

    from = s[0] - 'a';
    from += 8 * (8 - (s[1] - '0'));
    to = s[2] - 'a';
    to += 8 * (8 - (s[3] - '0'));

    for (int i = 0; i < workq.size(); i++) {
        if (workq[i].m.b.from == from && workq[i].m.b.to == to) {
            if (workq[i].m.b.bits & 32)
                switch (s[4]) {
                    case 'N':
                        if (workq[i].m.b.promote == KNIGHT)
                            return workq[i].m.u;
                    case 'B':
                        if (workq[i].m.b.promote == BISHOP)
                            return workq[i].m.u;
                    case 'R':
                        if (workq[i].m.b.promote == ROOK)
                            return workq[i].m.u;
                    default:
                        return workq[i].m.u;
                }
            return workq[i].m.u;
        }
    }

    /* didn't find the move */
    return -1;
}


/* move_str returns a string with move m in coordinate notation */

char *move_str(move_bytes m)
{
    static char str[6];

    char c;

    if (m.bits & 32) {
        switch (m.promote) {
            case KNIGHT:
                c = 'n';
                break;
            case BISHOP:
                c = 'b';
                break;
            case ROOK:
                c = 'r';
                break;
            default:
                c = 'q';
                break;
        }
        sprintf(str, "%c%d%c%d%c",
                COL(m.from) + 'a',
                8 - ROW(m.from),
                COL(m.to) + 'a',
                8 - ROW(m.to),
                c);
    }
    else
        sprintf(str, "%c%d%c%d",
                COL(m.from) + 'a',
                8 - ROW(m.from),
                COL(m.to) + 'a',
                8 - ROW(m.to));
    return str;
}


/* print_board() prints the board */

void print_board(node_t& board, FILE *stream)
{
    int i;

    fprintf(stream, "\n8 ");
    for (i = 0; i < 64; ++i) {
        switch (board.color[i]) {
            case EMPTY:
                fprintf(stream, " .");
                break;
            case LIGHT:
                fprintf(stream, " %c", piece_char[board.piece[i]]);
                break;
            case DARK:
                fprintf(stream, " %c", piece_char[board.piece[i]] + ('a' - 'A'));
                break;
        }
        if ((i + 1) % 8 == 0 && i != 63)
            fprintf(stream, "\n%d ", 7 - ROW(i));
    }
    fprintf(stream, "\n\n   a b c d e f g h\n\n");
}


/* print_result() checks to see if the game is over, and if so,
   prints the result. */

int print_result(std::vector<gen_t>& workq, node_t& board)
{
    int i;

    /* is there a legal move? */
    for (i = 0; i < workq.size() ; ++i) { 
        if (makemove(board, workq[i].m.b)) {
            takeback(board);
            break;
        }
    }
    if (i == workq.size()) {
        if (in_check(board, board.side)) {
            if (board.side == LIGHT)
                std::cout << "0-1 {Black mates}" << std::endl;
            else
                std::cout << "1-0 {White mates}" << std::endl;
        }
        else
            std::cout << "1/2-1/2 {Stalemate}" << std::endl;
        return 0;
    }
    else if (reps(board) == 3)
    {
        std::cout << "1/2-1/2 {Draw by repetition}" << std::endl;
        return 0;
    }
    else if (board.fifty >= 100)
    {
        std::cout << "1/2-1/2 {Draw by fifty move rule}" << std::endl;
        return 0;
    }
    return 1;
}

int parseArgs(int argc, char **argv)
{
    if (argc == 1)
    {
        return 0;
    }
    option_t *optList, *thisOpt;
    optList = NULL;
    optList = GetOptList(argc, argv,"w:b:mxoh?");
    int flag = 0;

    while (optList != NULL)
    {
        thisOpt = optList;
        optList = optList->next;

        if (('?' == thisOpt->option)||('h' == thisOpt->option))
        {
            // display help
            printf("Command line options:\n");
            printf("-h          Displays this help message\n");
            printf("-w n        Sets the depth of the white side to n (default 3)\n");
            printf("-b n        Sets the depth of the black side to n (default 3)\n");
            printf("-o          Turns off all engine output\n");
            printf("\n");
            FreeOptList(thisOpt);
            exit(0);
        }

        switch (thisOpt->option)
        {
            case 'w':
                // white depth level
                depth[LIGHT] = atoi(thisOpt->argument);
                if (depth[LIGHT] <= 0)
                {
                    fprintf(stderr, "Illegal depth given, setting depth to 1.\n");
                    depth[LIGHT] = 1;
                }
                flag = 1;
                break;
            case 'b':
                // black depth level
                depth[DARK] = atoi(thisOpt->argument);
                if (depth[DARK] <= 0)
                {
                    fprintf(stderr, "Illegal depth given, setting depth to 1.\n");
                    depth[DARK] = 1;
                }
                flag = 1;
                break;
            case 'o':
                output = 0;
                flag = 1;
                break;
        }
        free(thisOpt);
    }
    return flag;
}