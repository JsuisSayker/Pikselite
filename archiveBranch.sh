#! /usr/bin/bash

# Take the first argument as the branch name

branchName=$1

# Check if there are more or less than one argument

if [ $# -ne 1 ]
then
    echo "Please provide only one argument (the branch name you want to archive)"
    exit 1
fi

# Check if the branch you want to archive exists

if [ "$(git branch --list $branchName)" ]
then
    echo "The branch $branchName exists"
else
    echo "The branch $branchName does not exist"
    exit 1
fi

# Check if you are on the branch you want to archive

currentBranch=$(git branch --show-current)

if [ $currentBranch == $branchName ]
then
    echo "You are on the branch you want to archive (switch to the branch you want to merge the changes into)"
    exit 1
fi

#

# Create a tag named similarly to the branch but with the 'archive/' prefix
# Point it to the latest commit of the branch

git tag archive/$branchName $branchName

# Delete the branch locally

git branch -D $branchName

# Delete the local reference of remote branch ("forget" the remote branch)

git branch -dr origin/$branchName

# Push local tags to remote

git push origin --tags

# Delete the remote branch

git push -d origin $branchName
echo "The branch $branchName has been archived successfully"