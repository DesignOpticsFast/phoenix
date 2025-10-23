# Starter tests — wire into real fixtures.
import math

def _finite(col):
    return all((v is not None) and math.isfinite(v) for v in col)

def test_mtf_invariants(mtf_table):
    mtf = mtf_table.column('mtf').to_pylist()
    freq = mtf_table.column('frequency_cyc_per_mm').to_pylist()
    assert _finite(mtf) and _finite(freq)
    assert all(0.0 <= v <= 1.0 for v in mtf)
    assert all(v >= 0.0 for v in freq)
