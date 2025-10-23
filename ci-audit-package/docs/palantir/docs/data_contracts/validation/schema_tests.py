# Starter tests — replace with project Arrow imports and fixtures.
import math

def test_has_phoenix_version(arrow_table):
    md = getattr(arrow_table.schema, "metadata", {}) or {}
    assert b'phoenix_version' in md or 'phoenix_version' in md, "Missing phoenix_version"

def test_units_present_for_axes(arrow_table):
    md = getattr(arrow_table.schema, "metadata", {}) or {}
    required = [b'units:frequency_cyc_per_mm', b'units:mtf']
    missing = [k for k in required if (isinstance(md, dict) and k not in md)]
    assert not missing, f"Missing units: {missing}"
