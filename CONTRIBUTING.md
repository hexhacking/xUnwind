# Contributing to xUnwind

Welcome to the xUnwind project. Read on to learn more about our development process and how to propose bug fixes and improvements.

## Issues

We use GitHub issues to track public bugs and feature requests. Before creating an issue, please note the following:

1. Please search existing issues before creating a new one.
2. Please ensure your description is clear and has sufficient instructions to be able to reproduce the issue. The more information the better.


## Branch Management

There are 2 main branches:

1. `master` branch

    * It's the latest (pre-)release branch. We use `master` for tags.
    * **Please do NOT submit any PR on `master` branch.**

2. `dev` branch

    * It's our stable developing branch.
    * Once `dev` has passed our internal tests, it will be merged to `master` branch for the next release.
    * **Please always submit PR on `dev` branch.**


## Pull Requests

**If possible, always create an issue first where you can describe your problem or feature request. We can also communicate in this issue. Then you can link to that issue in the pull request.**

Please make sure the following is done when submitting a pull request:

1. Fork the repo and create your branch from `master`.
2. Add the copyright notice to the top of any new files you've added.
3. Check your Java code lints and checkstyles.
4. Try your best to test your code.
5. Squash all of your commits into one meaningful commit.


## Code Style Guide

1. Follow the [.clang-format](.clang-format) for C code.
2. Follow the [checkstyle.xml](checkstyle.xml) for Java code.


## License

By contributing to xUnwind, you agree that your contributions will be licensed under its [MIT LICENSE](LICENSE).
