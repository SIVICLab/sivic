#!/usr/bin/perl
use Cwd;
use Getopt::Long;
use Pod::Usage;
use strict;

sub check_repository_status($);
sub get_last_revision();
sub create_git_tag($);

Getopt::Long::config("bundling");
GetOptions(
    "i=s"         =>\my $index_to_increment,
    "b=s"         =>\my $tag_branch,
    "s"           =>\my $skip_repo_check,
    "c"           =>\my $create_and_push,
    "h"           =>\my $help,
    "v"           =>\my $verbose
) or pod2usage(-verbose=>2, -exitval=>2);

if( defined($help) ) {
	pod2usage(-verbose=>2, -exitval=>2);
}

if( !defined($index_to_increment) ) {
    $index_to_increment = 2;
}

if( !defined($tag_branch)) {
    $tag_branch = "develop";
}


###############################################################################
#  Make sure there are no local changes
###############################################################################
if( !defined($skip_repo_check) ) {
    my $repo_status = check_repository_status($tag_branch);
    if( $repo_status != 0 ) {
        print "ERROR: Your local repository is not suitable for tagging.\n";
        print "       You must use the $tag_branch branch AND have no local changes.\n";
        exit($repo_status);
    }
}

my @revision = get_last_revision();

# Permit the adding of one new index.
my $max_index = $#revision + 1;

if( $index_to_increment > $max_index || $index_to_increment < 0 ) {
    print "ERROR: Index ($index_to_increment) is out of range. Acceptable indices are 0-$max_index!\n";
    exit(1);
} elsif( $index_to_increment == $max_index ) {
    # Set the new index 0. It will be immediately incremented to 1 below.
    push(@revision, 0);
}

# increment the correct indices
for( my $i = $index_to_increment; $i <= $#revision; $i++ ) {
	if( $i == $index_to_increment ) {
		$revision[$i]++;
	} else {
		$revision[$i] = 0;
	}
}

print "New Revision: @revision\n";
my $new_tag = join('.', @revision );
print "New tag: $new_tag\n";

if( defined($create_and_push) ) {
    create_git_tag($new_tag);
}

exit(0);


###############################################################################
#
# SUBROUTINES
#
###############################################################################


#
# Checks to see if there are any local changes in your git repository.
# Also ensures that the user is on the correct branch.
#
sub check_repository_status($) 
{
    my( $tag_branch ) = @_;
    my $return_status = 0;
    # First let's check to make sure the user is on the correct branch.
    my $branch =`git rev-parse --abbrev-ref HEAD`;
    chomp($branch);
    if( $branch ne $tag_branch ) {
        print "ERROR: You are currently on branch $branch. Please checkout the $tag_branch branch.\n";
        $return_status = 1;
        return $return_status;
    }
    my $local_changes = `git diff origin/$tag_branch`;
    if( $local_changes ne "" ) {
        print "ERROR: You have local changes. Please commit and push your current changes.\n";
        $return_status = 1;
    }
    return $return_status;    
}


#
#  Gets the last tagged revision
#
sub get_last_revision() {
	
    ###############################################################################
    #  Parse to get the current revision
    ###############################################################################
    my @tags = `git ls-remote -t`;
    my $last_revision_commit;
    my @last_revision_number;
    foreach my $tag (@tags) {
    	if( $tag =~/(.*)\s+refs\/tags\/(.*)/ ) {
    		my $revision_commit = $1;
            my @revision_number = ($2 =~ /(\d+)/g);
            for (my $index = 0; $index <= $#revision_number; $index++ ) {
                # Assume an extra index is always later. Example 2.2.1 comes after 2.2.
                if( $index > $#last_revision_number ){
                    @last_revision_number = @revision_number;
                    $last_revision_commit = $revision_commit;
                    last;
                } elsif( $revision_number[$index] > $last_revision_number[$index] || $index == $#revision_number ) {
                    @last_revision_number = @revision_number;
                    $last_revision_commit = $revision_commit;
                    last;
                }
            }
    	}
    }

    if( !defined($last_revision_commit ) ) {
        print "ERROR: Could not find any previously tagged revisions.\n";
        exit(1);
    }
    print "Last Version: @last_revision_number\n";
    my $current_commit = `git rev-parse HEAD`;
    chomp($current_commit);
    if( $current_commit eq $last_revision_commit ) {
    	print "WARNING: Your current revision has already been tagged. No new tag created.\n";
    	exit(0);
    }
    return @last_revision_number;
}


#
#  Creates the new git tag.
#
sub create_git_tag($)
{
    my( $new_tag ) = @_;
    print "Creating tag: $new_tag\n";
    if( system("git tag $new_tag") != 0 ) {
        print "ERROR: Could not create git tag: $new_tag.\n";
        exit(1);
    } 
    if( system("git push origin $new_tag") != 0 ) {
        print "ERROR: Could not push git tag: $new_tag.\n";
        exit(1);
    }
     
}

###############################################################################
#
#   POD usage docs
#
###############################################################################

=head1 NAME

tag_revision.pl

=head1 SYNOPSIS
    
    tag_revision.pl [-i index -b branch -c -s -v]

        -c                 Create and push the tag to the remote repository. 
        -i  index          The revision index to increment.
                           (default=2)

        -b  branch         The branch that tags should be made on.
        				   (default=develop)

        -s                 Skip repository check for branch and local changes.
        -v                 Verbose output.
        -h                 Print this message.

=head1 DESCRIPTION 

	This script uses git tags to identify and increment version numbers for
	the current working directory. It will start by doing a check to ensure
	that the current working directory is on the specified branch for version
	tagging and that the user has no local changes. The -s flag can be used
	to bypass this check. It assumes that each git tag associated with a
	revision is a sequence of integers separated by a period. For example
	"0.9.4". It will then increment the specified index, using the previous
	example setting -i 2 will result in a new tag of "0.9.5" and setting -i 1
	will result in a new tag of "0.10.0".
	
	NOTE: By default the new tag will just be printed. To create and push
	the tag to the remote repository use the -c flag.

=cut

