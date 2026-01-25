# C++ Documentation Generator

Docker-based documentation generation with:
- **Doxygen** - API documentation, call graphs
- **clang-uml** - Sequence and class diagrams
- **PlantUML** - UML rendering
- **Graphviz** - Dependency graphs

## Quick Start

```bash
# Build the Docker image
./build.sh

# Generate all documentation (UML + Doxygen)
./docs.sh

# Preview in browser
./serve.sh
# Open http://localhost:8080
```

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
