# Changelog: Feature/Mixed-Collections

This branch introduces the ability to create "Mixed Collections" where a single list can contain games from multiple different systems, launching them with their respective emulators and displaying correct metadata.

## [feature/mixed-collections]

### Added
- **Mixed-System Syntax**: Support for `SystemName:GameName` syntax in `.sub` files (e.g., `Nintendo Entertainment System:Super Mario Bros`).
- **Dynamic Collection Generation**: The engine now creates lightweight `CollectionInfo` objects on-the-fly for foreign systems found in a list.
- **Auto-Import Settings**: Automatically imports `settings.conf` for foreign systems to ensure artwork paths (screenshots, videos) and launcher configurations are resolved correctly.
- **Single-Item Metadata Injection**: Implemented `MetadataDatabase::injectItemMetadata` to query the HyperList database for a specific game without requiring the full collection to be loaded.
- **File Parsing Improvements**:
    - **BOM Stripping**: Automatically detects and removes UTF-8 Byte Order Marks (BOM) from the start of `.sub` files to prevent parsing errors on the first line.
    - **Whitespace Trimming**: Trims leading/trailing whitespace from system and game names to ensure reliable database matching.

### Fixed
- Fixed memory leaks by tracking and deleting dynamically created `CollectionInfo` objects in the main collection's destructor.
- Fixed "missing artwork" issues by ensuring the foreign system's `settings.conf` is loaded into the global configuration.