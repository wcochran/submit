#!/usr/bin/perl

#
# Converts class roster downloaded from zzusis
# into "comma seperated value" (CSV) form.
# The downloaded file is suppose to be a
# MS Excel file, but it's actually HTML (Excel
# doesn't seem to be able to read it.
#
# usage:
#   ./roster.pl < downloaded.html > roster.csv
#

use strict;

my $nameNext = 0;
my $emailNext = 0;

for (<>) {
    next unless m{<td>(.*)</td>};
    my $data = $1;
    if ($data =~ /^(\d+)$/) {
        print "$data,";
        $nameNext = 1;
    } elsif ($nameNext) {
        print "\"$data\",";
        $nameNext = 0;
        $emailNext = 1;
    } elsif ($emailNext) {
        print "\"$data\"\n";
        $emailNext = 0
    }
}
