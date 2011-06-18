use strict;
use FileHandle;

my $fsup = 0;
my $asup = 0;
my $nsup = 0;
my $msup = 1;

my $anskey = {};
my $scoretab = {};
my $speedtab = {};

#Answer key:

#board1
#ply | answer
#====+=======
#  7 |   e2e4
#  6 |   b1c3
#  5 |   b2b4
#  4 |   g1f3
#  3 |   g1f3
#  2 |   d2d4

$anskey->{1}->{2} = "e2e4";
$anskey->{1}->{3} = "e2e4";
$anskey->{1}->{4} = "d2d4";
$anskey->{1}->{5} = "g1f3";

#board2
#ply | answer
#====+=======
#  7 |   d1b3
#  6 |   c1d2
#  5 |   c1g5
#  4 |   c1g5
#  3 |   c1d2
#  2 |   e2e4

$anskey->{2}->{2} = "e2e4";
$anskey->{2}->{3} = "e1d2";
$anskey->{2}->{4} = "c1d2";
$anskey->{2}->{5} = "c1g5";

#board3
#ply | answer
#====+=======
#  7 |   f5h7
#  6 |   f5g7
#  5 |   f5h6
#  4 |   f5g7
#  3 |   f3b3
#  2 |   b2b3

$anskey->{3}->{2} = "f5g7";
$anskey->{3}->{3} = "b2b3";
$anskey->{3}->{4} = "f3h5";
$anskey->{3}->{5} = "g5g6";

#board4
#ply | answer
#====+=======
#  7 |   e5f5
#  6 |   e5e3
#  5 |   h5h6
#  4 |   e5e3
#  3 |   e5b5
#  2 |   e5e3

$anskey->{4}->{2} = "e5a5";
$anskey->{4}->{3} = "g5f7";
$anskey->{4}->{4} = "g5f7";
$anskey->{4}->{5} = "g5f7";

my $bad_score = -6666;

for my $sm (("minimax","alphabeta","mtd-f")) {
    for(my $b=1;$b<=4;$b++) {
        for(my $ply=2;$ply<=5;$ply++) {
            # It takes too long for minimax above ply 4
            # So I ran it once at 5 to verify the answer
            # and then introduced this next.
            if($sm eq "minimax" and $ply >= 0) {
                next;
            }
            genbench($sm,$b,$ply);
            my $fd = new FileHandle;
            if($sm eq "mtd-f") {
                #sleep(1);
            }
            open($fd,"mpiexec -np 2 -env CHX_THREADS_PER_PROC 2 $ENV{PWD}/src/chx -s .bench.ini 2>/dev/null|");

            my $ans = "";
            my $score = $bad_score;

            my $st = "OK ";
            my $tm = 1e-6;
            my $speedup = 1;
            my $doc = "";
            while(<$fd>) {
                $doc .= $_;
                if(/Computer's move: ([a-h][1-8][a-h][1-8])/) {
                    if($ans ne "" and $ans ne $1) {
                        $st = "VARIABLE($ans != $1) ";
                        #last;
                    }
                    $ans = $1;
                }
                if(/SCORE=(-?\d+)/) {
                    if($score != $bad_score and $score != $1) {
                        $st = "VARIABLE($score != $1) ";
                        #last;
                    }
                    $score=$1;
                }
                if(/Average time for run: (\d+) ms/) {
                    $tm = $1*1.0e-3;
                    $tm = 1.0e-3 if($tm < 1.0e-3);
                }
            }
            if(defined($anskey->{$b}->{$ply})) {
                my $v = $anskey->{$b}->{$ply};
                if($ans ne $v) {
                    $st .= "MOVE($v) ";
                }
            }
            if(defined($scoretab->{$b}->{$ply})) {
                my $s = $scoretab->{$b}->{$ply};
                if($s != $score) {
                    $st .= "SCORE($s) ";
                }
            }
            if($st eq "OK ") {
                $scoretab->{$b}->{$ply} = $score;
            }
            if(defined($speedtab->{$b}->{$ply})) {
                $speedup = $speedtab->{$b}->{$ply}/$tm;
            }
            $speedtab->{$b}->{$ply} = $tm;
            printf("%10s, %4d, %4d, %9d, %s, %s, %.2f\n",$sm,$b,$ply,$score,$ans,$st,$speedup);
            if($sm eq "mtd-f") {
                $fsup += 1 if($speedup > 1);
                $fsup += 0.5 if($speedup == 1);
                $asup += $speedup;
                $nsup += 1;
                $msup *= $speedup;
            }
            my $ret=close($fd);
            unless($ret) {
                print $doc;
                die "error code returned r=($ret)";
            }
        }
    }
}

printf("mtd-f over alpha speedup: avg=%3.2f geomtric mean=%3.2f fraction faster=%3.2f\n",$asup/$nsup,($msup)**(1.0/$nsup),$fsup/$nsup);

sub genbench {
    my $sm = shift;
    my $b = shift;
    my $ply = shift;
    my $fd = new FileHandle;
    my $runs = 3;
    $runs = 1 if($sm eq "minimax");
    $runs = 1 if($ply > 5);
    open($fd,">.bench.ini");
print $fd qq{
; settings.ini
; Configuration file for CHX

[CHX Main]

; Valid arguments to search_method:
;  minimax (default)
; alphabeta
;search_method = mtd-f
search_method = $sm

; Valid arguments to eval_method:
;  original (default)
;  simple
eval_method = original

; Valid arguments to num_threads: (note that this only works when OpenMP is enabled, otherwise ignored)
;  [1-9][0-9]* (default 3)
num_threads = 2

; Valid arguments to output:
;  true (default)
;  false
output = true

; Iterative deepening can be tuned to only go to a certain number of plys before just calling the
; search function
; Valid arguments to iter_depth:
; [1-9][0-9]* (default 5)
iter_depth = 5

[Depth]

; This sets the ply-depth of the white player
; Valid arguments to white:
; [1-9][0-9]* (default 3)
white = 7

; Depth for black player, same arguments
black = 7

[Benchmark]
; This section is for using the built in benchmark

; If mode is set to true, then it will read in the file
; specified by file, run the benchmark and then quit (useful for automating the bench runs)
mode=true

; If a relative path is given, it will be to the relative path in which CHX was called from
file=inputs/board$b

max_ply=$ply

parallel=true

num_runs=$runs
};
    close($fd);
}
