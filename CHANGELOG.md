# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2026-09-09

### Added

- Vamp plugin exposing the features as audio analysis plugins for hosts such as Sonic Visualiser, Audacity and Sonic Annotator.
  Prebuilt binaries for Linux, macOS and Windows are attached to the release.
- Python 3.14 support.

### Changed

- Rename the Python distribution to `openae-core` and publish it to PyPI.
  The import name is unchanged: `pip install openae-core`, then `import openae.features`.
- Make `openae` a PEP 420 namespace package to allow future `openae.*` packages.

### Removed

- Python 3.8 support.

### Fixed

- `partial-power`: floor instead of round frequencies to spectrum bins.

## [0.1.0] - 2025-03-20

Initial public release.

[unreleased]: https://github.com/openae-io/openae-lib/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/openae-io/openae-lib/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/openae-io/openae-lib/releases/tag/v0.1.0
