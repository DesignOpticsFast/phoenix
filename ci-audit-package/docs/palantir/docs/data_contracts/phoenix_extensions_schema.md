# Phoenix Extensions for Vega-Lite

**Purpose:** Optional `phoenix` object embedded in Vega-Lite specs to carry domain, rendering, theme, interactivity, and provenance hints.

```ts
interface PhoenixExtensions {
  version: string;                 // e.g., "1.0.0"
  optical?: OpticalMetadata;
  rendering?: RenderingHints;
  theme?: ThemeOverrides;
  interactivity?: InteractionConfig;
  provenance?: ProvenanceInfo;
}

interface OpticalMetadata {
  units: Record<string,string>;    // {"frequency_cyc_per_mm":"cycles/mm"}
  wavelengths?: number[];          // nm
  fields?: Array<{x_deg?:number,y_deg?:number,label?:string}>;
  aperture?: {type:"circular"|"annular", diameter_mm?:number, epsilon?:number};
  system_id?: string;
}

interface RenderingHints {
  decimation_strategy?: "LTTB"|"UNIFORM"|"PERCEPTUAL";
  lod_enabled?: boolean;
  webgl_fallback?: boolean;
  max_points?: number;
}

interface ThemeOverrides {
  base?: "phoenix_dark"|"phoenix_light";
  overrides?: Record<string,string>; // token -> color/hex
}

interface InteractionConfig {
  tooltip_format?: "optical_standard"|"raw";
  zoom_constraints?: {x?:[number,number], y?:[number,number]};
  export_actions?: Array<"svg"|"pdf"|"png"|"csv">;
}

interface ProvenanceInfo {
  bedrock_version: string;
  calc_time: string;               // ISO8601
  config_hash: string;
}
```

**Validation:** Phoenix validates `phoenix.version` and known fields; unknown fields are ignored for forward compatibility.
