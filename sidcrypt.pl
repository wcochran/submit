#!/usr/bin/perl

#
# Hash WSU student ID's with UNIX crypt.
# Each line of the input text file that begins
# with a sequence of 8 digits is encrypted
# and output. If the input string is 9 digits
# with a leading 0, the leading 0 is stripped.
#
# usage:
#   ./sidcrypt.pl < sid.csv > sidcrypt.txt
#

use strict;

for (<>) {
    next unless /^(\d+)/;
    my $sid = $1;
    $sid = substr($sid, 1) if $sid =~ /^0/; # strip leading 0
    if (length($sid) != 8) {
	print STDERR "$sid Not 8 chars ... skipping\n";
	next;
    }
    my $salt = join '', ('.', '/', 0..9, 'A'..'Z', 'a'..'z')[rand 64, rand 64];
    my $cypher = crypt $sid, $salt;
    print $cypher, "\n";
}
