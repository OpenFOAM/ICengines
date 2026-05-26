#!/bin/bash
# setup-remotes.sh - Configure remotes for public branch workflow

GITHUB_URL="https://github.com/OpenFOAM/ICengines.git"
BITBUCKET_URL="ssh://git-ssh.devops.wartsila.com:7999/ope/aate"

# Check if remotes exist, add if not
if ! git remote get-url github > /dev/null 2>&1; then
    echo "Adding GitHub remote..."
    git remote add github "$GITHUB_URL"
fi

if ! git remote get-url origin > /dev/null 2>&1; then
    echo "Adding BitBucket remote as origin..."
    git remote add origin "$BITBUCKET_URL"
fi

# Enable repository hooks for this clone
if [ -d ".githooks" ]; then
    echo "Configuring local hooks path to .githooks..."
    git config core.hooksPath .githooks
    chmod +x .githooks/pre-commit .githooks/pre-push 2>/dev/null || true
fi

# If on public branch, set both pull and push target to GitHub master
if [ "$(git rev-parse --abbrev-ref HEAD)" = "public" ]; then
    echo "Fetching from GitHub..."
    git fetch github

    echo "Setting public branch to:"
    echo "  - Pull from: github/master"
    echo "  - Push to: github/master"

    # Set upstream (pull) to github/master
    git branch --set-upstream-to=github/master public

    # Set push remote to github and push behavior to upstream branch
    git config branch.public.pushRemote github
    git config push.default upstream
fi

echo "Remotes configured:"
git remote -v