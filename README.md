# PIXEL PFS — iOS cloud build scaffold

This repository contains the supplied C++17/SDL2 game source and a GitHub Actions workflow for an **unsigned iOS build**.

## Important limitation
The workflow packages an **unsigned IPA**. It is useful as a build artifact, but it normally cannot be installed on a real iPhone until it is signed with a matching Apple development/distribution certificate and provisioning profile. A `.p12` file alone is not enough; you normally also need the matching `.mobileprovision` profile and its bundle ID/device entitlement details. Do not upload certificate passwords or private signing files to a public repository.

## Create the GitHub repository using an iPhone
1. Open https://github.com/new in Safari and sign in.
2. Repository name: `pixel-pfs-ios`.
3. Choose **Private** (recommended), then tap **Create repository**.
4. Download this ZIP, extract it using the Files app, and upload the files/folders into the repository. Keep `.github/workflows/build-ios.yml` at exactly that path.
5. Commit the files to the `main` branch.
6. Open the repository's **Actions** tab. If prompted, enable workflows.
7. Choose **Build PIXEL PFS iOS** → **Run workflow**. You can also trigger it by pushing a commit.
8. When the run finishes, open the run, scroll to **Artifacts**, and download `PixelPFS-iOS-unsigned`.

## If the workflow fails
This is a starter scaffold, not a guarantee that the current SDL2 CMake/Xcode combination will build without adjustment. Open the failed workflow and share the first red error lines; those identify the exact fix needed. The source currently uses SDL2 APIs and should be compiled as C++17.

## Signing later
For a real-device install, configure signing privately in GitHub Actions using repository secrets and a matching provisioning profile. Do not put `.p12`, profile files, passwords, API keys, or other signing secrets directly in source files or a public repository.
