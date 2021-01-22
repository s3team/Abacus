#!/usr/bin/perl

use strict; 
use warnings;
use Time::HiRes qw (time);

my $PIN_CMD="/home/gregz/work/malware/pin-3.6-97554-g31f0a167d-gcc-linux/pin";
my $PIN_TOOL="/home/gregz/work/malware/pin-3.6-97554-g31f0a167d-gcc-linux/source/tools/MyPinTool/obj-ia32/MyPinTool.so";

my @programs;
if(defined($ARGV[0])) {
    if($ARGV[0] !~ /^\.\/.*/) {       # Make sure it starts with "./"
        $ARGV[0] = "./$ARGV[0]";        
    }
    @programs[0] = $ARGV[0];
}
else {
    print "Creating traces for all programs in current directory\n";
    my $find_output = `find . -maxdepth 1 -type f -executable`;
    @programs = split(/\n/, $find_output);
}

foreach my $program (@programs) {
    next if($program =~ /.*\.pl/);
    next if($program =~ /.*\.sh/);

    print "Creating trace for $program ... ";
    my $start = Time::HiRes::time();
    my $rv = system("$PIN_CMD -t $PIN_TOOL -- $program");
    if($rv == 0) {
        system("mv instrace.txt $program.trace");
    }
    my $end = Time::HiRes::time();
    print "Took " . ($end - $start) . " seconds\n";
}
