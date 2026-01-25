# C++ Documentation Generator

Docker-based documentation generation with:
- **Doxygen** - API documentation, call graphs
- **clang-uml** - Sequence and class diagrams
- **PlantUML** - UML rendering
- **Graphviz** - Dependency graphs

## 🎯 Reproducible Builds (RECOMMENDED)

This project supports **vendored dependencies** for fully reproducible, offline builds that will work identically even years from now.

### Quick Start (Vendored)

```bash
# 1. Download all dependencies (one-time setup)
./vendor.sh

# 2. Build with vendored dependencies (fully offline)
./build.sh vendored

# 3. Generate documentation
./docs.sh
```

### Why Vendor Dependencies?

✅ **Future-proof** - Works identically in 5+ years
✅ **Offline builds** - No internet required after initial vendor
✅ **Supply chain security** - All dependencies verified once
✅ **Reproducible** - Same input = same output, always
✅ **CI/CD friendly** - Fast, reliable builds

## 📚 Example Code Included

The `project/` directory includes example C++ code to test the documentation:

- **Calculator.h/cpp** - Simple calculator with method chaining
- **StringUtils.h/cpp** - String manipulation utilities
- **main.cpp** - Demo application using both classes

This example code will generate:
- ✅ Full API documentation with Doxygen
- ✅ Class diagrams showing both classes
- ✅ Call graphs and sequence diagrams
- ✅ Include dependency graphs

See `project/README.md` for details about the example code.

## Scripts

| Script | Description |
|--------|-------------|
| `vendor.sh` | **Download all dependencies for offline builds (run once)** |
| `build.sh` | Build Docker image (supports `vendored` mode) |
| `build-vendored.sh` | Build using only vendored dependencies |
| `docs.sh` | Generate everything (UML + Doxygen) |
| `doxygen.sh` | Doxygen only (faster) |
| `uml.sh` | UML diagrams only |
| `shell.sh` | Interactive shell |
| `serve.sh [port]` | Serve docs locally (default: 8080) |

## Dependency Management

### Vendoring Workflow

```bash
# Initial setup (one time)
./vendor.sh                    # Downloads all dependencies (~500MB)

# Build and use
./build.sh vendored            # Build from vendored deps
./docs.sh                      # Generate documentation

# 5 years later... (same commands work perfectly)
./build.sh vendored            # Identical build, no internet needed
```

### Storage Options

The `vendored/` directory contains all dependencies (~500MB-1GB):

**Option 1: Git LFS** (Recommended for teams)
```bash
git lfs track "vendored/**/*"
git add .gitattributes vendored/
git commit -m "Add vendored dependencies"
```

**Option 2: Separate Artifact Storage** (Recommended for CI/CD)
- Upload `vendored/` to artifact repository (Artifactory, Nexus)
- Download in CI pipeline before build
- Keep git repo lightweight

**Option 3: Version Control** (Small teams)
```bash
# Remove from .gitignore, commit directly
git add vendored/
git commit -m "Vendor dependencies for reproducibility"
```

**Option 4: Archive** (Long-term preservation)
```bash
tar -czf vendored-backup-$(date +%Y%m%d).tar.gz vendored/
# Store in secure backup location
```

### Vendored Dependencies

When you run `./vendor.sh`, it downloads and stores:

- **PlantUML** JAR (v1.2024.0)
- **clang-uml** source (v0.5.4)
- **Ubuntu** base image (22.04)
- **APT packages** (~200 .deb files for build tools, Doxygen, Graphviz, Clang)

All stored in `vendored/` directory with SHA256 checksums in `vendored/MANIFEST.txt`.

## Scripts

| Script | Description |
|--------|-------------|
| `build.sh` | Build the Docker image |
| `docs.sh` | Generate everything (UML + Doxygen) |
| `doxygen.sh` | Doxygen only (faster) |
| `uml.sh` | UML diagrams only |
| `shell.sh` | Interactive shell |
| `serve.sh [port]` | Serve docs locally (default: 8080) |

## Project Structure

```
.
├── Dockerfile
├── generate-docs.sh
├── build.sh
├── docs.sh
├── doxygen.sh
├── uml.sh
├── shell.sh
├── serve.sh
└── project/
    ├── CMakeLists.txt
    ├── Doxyfile
    ├── .clang-uml
    ├── include/
    ├── src/
    └── docs/
        ├── html/          # Doxygen HTML output
        ├── xml/           # XML for PLM import
        └── uml/           # UML diagrams
```

## Output

| Directory | Contents |
|-----------|----------|
| `docs/html/` | Doxygen HTML (open index.html) |
| `docs/xml/` | XML for PLM import (Polarion, PTC) |
| `docs/uml/` | PlantUML sources + rendered SVG/PNG |

## Manual Docker Commands

```bash
# Build image
docker build -t cpp-docs .

# Generate all docs
docker run --rm -v $(pwd)/project:/project -w /project cpp-docs generate-docs

# Doxygen only
docker run --rm -v $(pwd)/project:/project -w /project cpp-docs doxygen Doxyfile

# Interactive
docker run --rm -it -v $(pwd)/project:/project -w /project cpp-docs bash
```

## Configuration

### Doxyfile
Main Doxygen configuration. Key settings:
- `EXTRACT_ALL = YES` - Extract even undocumented code
- `CALL_GRAPH = YES` - Generate call graphs
- `GENERATE_XML = YES` - For PLM import

### .clang-uml
UML diagram configuration:
- `class_overview` - Full class diagram
- `sequence_main` - Main function sequence
- `include_dependencies` - Header dependencies

## PLM Export

XML output in `docs/xml/` can be imported into:
- Polarion
- PTC Windchill / Integrity
- IBM DOORS

## Tools Included

- Doxygen
- Graphviz
- PlantUML
- clang-uml
- Clang 15
