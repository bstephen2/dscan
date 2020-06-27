#!/bin/perl
use warnings;
use English '-no_match_vars';
use strict;
use MP3::Tag;

our $VERSION = 1.1;

my $fname = $ARGV[0];
my $rc;
my $mp3 = MP3::Tag->new($fname);
my (
    $mp3_title,   $mp3_track, $mp3_artist, $mp3_album,
    $mp3_comment, $mp3_year,  $mp3_genre
) = $mp3->autoinfo();

my $artist  = $mp3->artist();
my $bitrate = $mp3->bitrate_kbps();
my $tsecs   = $mp3->total_secs();

$rc = print "$mp3_track\n";
$rc = print "$mp3_genre\n";
$rc = print "$bitrate\n";
$rc = print "$tsecs\n";
$rc = print "$artist\n";
$rc = print "$mp3_album\n";

exit 0;

