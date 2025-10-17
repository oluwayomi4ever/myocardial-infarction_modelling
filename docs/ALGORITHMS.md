# Core Algorithms in the MI Modeling C++ Project

This document summarizes the core algorithms implemented in the MI Modeling C++ Project and explains how they are applied to simulate, analyze, and validate myocardial infarction (MI) phenomena. It is intended as a concise, submission-ready technical description.

---

## 1) FitzHugh–Nagumo (FHN) Electrophysiology Model

- **Purpose**: Models cardiac tissue excitability and action potential propagation at tissue scale with low computational cost.
- **State variables**: `u` (membrane potential, fast variable) and `v` (recovery, slow variable).
- **Governing equations (reaction terms)**:
  - du/dt = u − u^3/3 − v + I_stim
  - dv/dt = (u + a − b·v)/c
- **Where used in code**:
  - Reaction terms and solver: `src/FitzHughNagumo.cpp`:
```246:254:/home/ned/projects/cpp/mcmodel/src/FitzHughNagumo.cpp
// Reaction terms
double FitzHughNagumo::reactionU(double u_val, double v_val, double stim_val) {
    return u_val - u_val * u_val * u_val / 3.0 - v_val + stim_val;
}

double FitzHughNagumo::reactionV(double u_val, double v_val) {
    return (u_val + a_ - b_ * v_val) / c_;
}
```
- **Time integration**: Explicit forward Euler time stepping on a regular grid:
```110:121:/home/ned/projects/cpp/mcmodel/src/FitzHughNagumo.cpp
// Euler update for each grid cell
u_new_[y][x] = u_val + dt_ * du_dt[y][x];
v_new_[y][x] = v_val + dt_ * dv_dt[y][x];
...
// advance time
time_ += dt_;
```
- **Diffusion (spatial coupling / propagation)**: 5‑point stencil Laplacian adds conduction-like spread:
```234:242:/home/ned/projects/cpp/mcmodel/src/FitzHughNagumo.cpp
// 5-point stencil Laplacian for diffusion
double laplacian = grid[y-1][x] + grid[y+1][x] + grid[y][x-1] + grid[y][x+1] - 4.0 * grid[y][x];
result[y][x] = coeff * laplacian;
```
- **MI relevance**: By tuning parameters `(a, b, c)` and diffusion coefficients `(du, dv)`, we emulate ischemic, border zone, and infarcted tissue (reduced excitability/conduction) to study conduction block, slowed propagation, and reentry-like patterns.

---

## 2) Diffusion Operator (Electrical Conduction Surrogate)

- **Purpose**: Approximates spatial current spread in tissue (monodomain-like behavior) without solving full bidomain PDEs.
- **Discretization**: Second-order central differences (5‑point stencil) on a 2D grid.
- **Stability**: Paired with explicit Euler; `dt` and diffusion coefficients must satisfy CFL-like constraints to avoid instability.
- **Where used**: Same implementation as above in `applyDiffusion(...)` within `FitzHughNagumo.cpp`.
- **MI relevance**: Reduced diffusion in infarcted zones models slowed/blocked conduction.

---

## 3) Differential Transform Method (DTM) Utilities

- **Purpose in this project**: Utility for grid-based field generation and spatial analysis (slope/aspect) and for demonstrating a transform-based workflow for differential problems.
- **Core operations**:
  - Read/write gridded fields (header + rows of values)
  - Neighborhood gradients and derived measures (slope, aspect)
- **Where used in code**: `src/DTM.cpp`:
```120:146:/home/ned/projects/cpp/mcmodel/src/DTM.cpp
// Slope/aspect via centered differences on a 3x3 neighborhood
double dz_dx = (getElevation(x + 1, y) - getElevation(x - 1, y)) / (2.0 * cell_size_);
double dz_dy = (getElevation(x, y + 1) - getElevation(x, y - 1)) / (2.0 * cell_size_);
...
return slope_radians * 180.0 / M_PI;  // slope in degrees
```
- **MI relevance**: The same stencil machinery and file IO patterns are used in electrophysiology to compute spatial derivatives and manage state fields. The “DTM” naming in this codebase represents a scaffold for transform/field-based operators that can be extended to cardiac parameter maps (e.g., conductivity, scar masks).

---

## 4) Numerical Integration Strategy

- **Scheme**: Explicit forward Euler in time, finite-difference in space.
- **Pros**: Simple, fast, easy to vectorize and port to GPU/WASM; great for interactive demos.
- **Cons**: Requires small `dt` for stability when diffusion is high; implicit/semi-implicit schemes can be adopted later if needed.
- **Code locations**: Time stepping (`step()` / `run()`), diffusion, and reaction are all in `src/FitzHughNagumo.cpp`.

---

## 5) WebAssembly (WASM) Execution Path (Browser)

- **Purpose**: Run the same C++ model directly in the browser with near‑native speed for interactive MI experiments.
- **Binding layer**: `webassembly/mi_modeling_wasm.cpp` exposes C++ classes/functions to JavaScript using Embind.
- **Build**: `webassembly/build_wasm.sh` compiles `FitzHughNagumo.cpp`, `DTM.cpp`, and bindings into `dist/mi_modeling.(js|wasm)`.
- **Use**: The UI (`webassembly/index.html`) loads the WASM module and invokes C++ APIs to initialize, stimulate, run, and visualize.
- **MI relevance**: Enables bedside/teaching demos and rapid hypothesis testing without server dependencies.

---

## 6) Data Processing (ECG example)

- **Goal**: Provide a minimal, end‑to‑end loop from uploaded data → metrics → model parametrization.
- **Operations** (in WASM/Python variants): high‑pass baseline removal, simple threshold‑based R‑peak detection, and summary stats (min/max/mean/std). These steer demo parameters and validate outputs.

---

## 7) Model Calibration Hooks (Parameters)

- **Electrophysiology**: `(a, b, c, d)` tune excitability/recovery; `(du, dv)` tune conduction.
- **Stimulus**: Position `(x, y)`, strength, and duration shape activation patterns.
- **Tissue Heterogeneity**: Region‑wise parameter maps (normal/ischemic/infarct). In practice, populate these from imaging (MRI LGE) or ECG‑inferred maps.

---

## 8) Limitations and Extensions

- **Simplified tissue model**: FHN omits ionic detail; upgrade path includes Luo–Rudy or Ten Tusscher cells coupled with monodomain/bidomain PDEs.
- **Explicit time stepping**: Consider semi‑implicit solvers (e.g., CN, IMEX) for larger `dt` and stiff regimes.
- **Anisotropy & Fiber fields**: Extend diffusion to tensor form for fiber‑aligned conduction.
- **3D & Real anatomy**: Port to 3D meshes and anatomical geometries; integrate segmentation pipelines.

---

## 9) How Algorithms Map to MI Questions

- **Conduction slowing/block**: Lower diffusion and altered `(a, b, c)` reproduce ischemic conduction defects and block.
- **Reentry propensity**: Spatial heterogeneity + reduced diffusion can create wave breaks in FHN.
- **ECG surrogates**: Aggregate membrane potential fields to derive pseudo‑ECG traces for qualitative validation.

---

## References
- FitzHugh R. (1961); Nagumo J. et al. (1962) – Excitable media model foundations.
- Standard 5‑point finite differences – Discrete Laplacian on Cartesian grids.
- Project docs: `docs/LITERATURE_REVIEW.md`, `docs/VALIDATION_REPORT.md`.
