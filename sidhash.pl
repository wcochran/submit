#!/usr/bin/perl

#
# This generates string used to "anonymously"
# associate posted scores and grades with a
# a student.
# Each student's 8-digit ID is hashed with SHA-256
# 150000 times and the last 7 hex digits
# are the result.
# This mimics the tool found at
# http://ezekiel.vancouver.wsu.edu/hashsid/hashsid.html
#

use strict;
use Digest::SHA qw(sha256_hex);

sub computeHash {
    my $sid = shift;
    my $hash = $sid;
    for (my $i = 0; $i < 150000; $i++) {
	$hash = sha256_hex($hash);
    }
    my $tail = substr $hash, -7;
    return uc $tail;
}

for (<>) {
    next unless /^(\d+)/;
    my $sid = $1;
    $sid = substr($sid, 1) if $sid =~ /^0/; # strip leading 0
    my $h = computeHash $sid;
    print "$h\n";
}
