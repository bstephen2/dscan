#!/bin/perl
use warnings;
use feature qw(switch say);
use English '-no_match_vars';
use strict;
use File::stat;
use Time::HiRes qw(gettimeofday);
use DBI;
use MP3::Tag;
use Readonly;

Readonly::Scalar my $DS_OUTPUT                   => 'c:\\bin\\bdserr';
Readonly::Scalar my $DS_INSERT_TRACK             => '0';
Readonly::Scalar my $DS_INVALID_AUTHOR           => '1';
Readonly::Scalar my $DS_MISPLACED_DIRECTORY      => '2';
Readonly::Scalar my $DS_MISPLACED_MP3            => '3';
Readonly::Scalar my $DS_INVALID_ADAPTOR          => '4';
Readonly::Scalar my $DS_INCONSISTENT_ALBUM_NAMES => '5';
Readonly::Scalar my $DS_SECONDS_PER_MINUTE       => 60.0;
Readonly::Scalar my $MAX_ERRORS                  => 10;
Readonly::Scalar my $DS_EPOCH_YEAR               => 1900;
Readonly::Scalar my $DS_SECONDS_PER_HOUR         => 3600;
Readonly::Scalar my $DS_COLON                    => q{:};

our $VERSION = 1.1;
my $ofile;
my $rc;
my @o_array;
my @s_array;
my $before = gettimeofday();
my $duration;
my $dbhandle = db_connect();
my $raw_authors;
my $album;
my $insert_track_format;

BEGIN {
    $insert_track_format =
        'INSERT INTO track SET '
      . 'alid = %d, '
      . 'name = "%s", '
      . 'date = "%s", '
      . 'trno = %d, '
      . 'genre = "%s", '
      . 'bitrate = "%s", '
      . 'length = "%s"';
}

while (<>) {
    my $line = $_;
    chomp $line;

    given ( substr $line, 0, 1 ) {
        when ($DS_INSERT_TRACK) {
            do_file($line);
        }

        when ($DS_INVALID_AUTHOR) {
            my $r_hash = {};
            $r_hash->{KEY} = substr $line, 2;
            $r_hash->{VALUE} = 'INVALID AUTHOR => ' . substr $line, 2;
            push @o_array, $r_hash;
        }

        when ($DS_MISPLACED_DIRECTORY) {
            my $r_hash = {};
            $r_hash->{KEY} = substr $line, 2;
            $r_hash->{VALUE} = 'MISPLACED DIRECTORY => ' . substr $line, 2;
            push @o_array, $r_hash;
        }

        when ($DS_MISPLACED_MP3) {
            my $r_hash = {};
            $r_hash->{KEY} = substr $line, 2;
            $r_hash->{VALUE} = 'MISPLACED_MP3 => ' . substr $line, 2;
            push @o_array, $r_hash;
        }

        when ($DS_INVALID_ADAPTOR) {
            my $r_hash = {};
            $r_hash->{KEY} = substr $line, 2;
            $r_hash->{VALUE} = 'INVALID ADAPTOR => ' . substr $line, 2;
            push @o_array, $r_hash;
        }

        when ($DS_INCONSISTENT_ALBUM_NAMES) {
            my $r_hash = {};
            $r_hash->{KEY} = substr $line, 2;
            $r_hash->{VALUE} = 'INCONSISTENT ALBUM NAMES => ' . substr $line, 2;
            push @o_array, $r_hash;
        }

        default {
            warn "INVALID LOG RECORD => $line\n";
        }
    }
}

disconnect($dbhandle);
@s_array = sort { $a->{KEY} cmp $b->{KEY} } @o_array;

## no critic (RequireBriefOpen)

( $rc = open $ofile, '>', $DS_OUTPUT ) or die "Can't open output\n";

foreach my $hash (@s_array) {
    $rc = printf {$ofile} "%s\n", $hash->{VALUE};
}

$rc = close $ofile;

## use critic

$duration = ( gettimeofday() - $before ) / $DS_SECONDS_PER_MINUTE;
warn "'Perl' time = $duration minutes\n";

exit 0;

sub do_file {
    my $path = shift;
    my ( $type, $url_len, $album_id, $drive, $pathname ) = split /:/xsm, $path;
    my $line = substr $pathname, $url_len - 2;
    my ( $author, $adaptor, $base_album, $raw_album, $fname ) =
      split /[\/\\]/xsm,
      $line;

    # Process tags
    $raw_authors = $author;
    $album       = $raw_album;
    my $r_hash = get_file_details( $drive . $DS_COLON . $pathname );

    # Insert track
    my $command;
    my $sth;

    $command = sprintf $insert_track_format,
      $album_id,
      $fname,
      $r_hash->{DATE},
      $r_hash->{TRNO},
      $r_hash->{GENRE},
      $r_hash->{BITRATE},
      $r_hash->{LEN};
    $sth = $dbhandle->prepare($command);
    $sth->execute();
    $sth->finish();

    return;
}

sub db_connect {
    my $server   = 'localhost';
    my $db       = 'drama';
    my $user     = 'bstephen';
    my $password = 'rice37';
    my $dbh;
    my $ct = 0;
    my %attr = ( PrintError => 0, RaiseError => 1 );

    while ( !defined $dbh ) {
        $dbh =
          DBI->connect( "dbi:mysql:$db:$server", $user, $password, \%attr );

        if ( !defined $dbh ) {
            $ct++;
            ( $ct == $MAX_ERRORS )
              && ( die "10 failed connect tries. Baling out\n" );
        }
    }

    return $dbh;
}

sub disconnect {
    my ($dbh) = @_;

    $dbh->disconnect();

    return;
}

sub get_file_details {
    my $pname  = shift;
    my $r_hash = {};

    my $sb = stat $pname;

    my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) =
      localtime $sb->mtime;

    $r_hash->{DATE} = sprintf '%04d-%02d-%02d', $year + $DS_EPOCH_YEAR,
      $mon + 1, $mday;

    my $mp3 = MP3::Tag->new($pname);
    my (
        $mp3_title,   $mp3_track, $mp3_artist, $mp3_album,
        $mp3_comment, $mp3_year,  $mp3_genre
    ) = $mp3->autoinfo();

    my $artist = $mp3->artist();

    if ( $raw_authors ne $artist ) {
        my $_r_hash = {};
        $r_hash->{KEY} = $pname;
        $r_hash->{VALUE} =
          "INCONSISTENT ARTISTS => ($album) $raw_authors === $artist";
        push @o_array, $r_hash;
    }

    if ( $album ne $mp3_album ) {
        my $_r_hash = {};
        $r_hash->{KEY} = $pname;
        $r_hash->{VALUE} =
          "INCONSISTENT ALBUMS ($raw_authors) $album === $mp3_album";
        push @o_array, $r_hash;
    }

    $r_hash->{TRNO}    = $mp3_track;
    $r_hash->{GENRE}   = $mp3_genre;
    $r_hash->{BITRATE} = $mp3->bitrate_kbps();
    my $tsecs = $mp3->total_secs();

    my $t_hours = int( $tsecs / $DS_SECONDS_PER_HOUR );
    my $t_rem1  = $tsecs % $DS_SECONDS_PER_HOUR;
    my $t_mins  = int( $t_rem1 / $DS_SECONDS_PER_MINUTE );
    my $t_secs  = $t_rem1 % $DS_SECONDS_PER_MINUTE;
    $r_hash->{LEN} = sprintf '%02d:%02d:%02d', $t_hours, $t_mins, $t_secs;

    return $r_hash;
}

