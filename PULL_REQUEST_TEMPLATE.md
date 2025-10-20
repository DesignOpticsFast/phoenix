# Pull Request Template

## Pre-submission Checklist

### Code Quality
- [ ] Code follows C++17 (Phoenix) / C++20 (Bedrock) standards
- [ ] All tests pass locally
- [ ] No compiler warnings or errors
- [ ] Code is properly formatted and documented

### Policy Compliance
- [ ] **No deprecated libraries or APIs** (e.g., Qt6WebEngine, Qt6Xml)
- [ ] **Build passes policy check** (policy.yml workflow)
- [ ] **Any exception has a signed ADR** (Architecture Decision Record)

### Dependencies
- [ ] No new deprecated dependencies introduced
- [ ] All dependencies are up-to-date and supported
- [ ] CMake policy enforcement passes

### Testing
- [ ] Unit tests added/updated as needed
- [ ] Integration tests pass
- [ ] Performance tests meet acceptance criteria
- [ ] Telemetry data collected and documented

### Documentation
- [ ] Code is self-documenting with clear comments
- [ ] API changes documented
- [ ] README updated if needed
- [ ] Changelog updated

## Policy Violations

### Deprecated Libraries
**ZERO TOLERANCE** for deprecated libraries:
- ✅ Qt Graphs (modern plotting library)
- ❌ Qt6WebEngine (use Qt6WebEngineCore)
- ❌ Qt6WebEngineWidgets (use Qt6WebEngineWidgets)
- ❌ Qt6WebView (use Qt6WebEngine)
- ❌ Qt6Xml (use Qt6Core)
- ❌ Qt6XmlPatterns (use Qt6Core)

### Build Requirements
- [ ] CMake policy enforcement passes
- [ ] No deprecated module detection
- [ ] QT_DISABLE_DEPRECATED_UP_TO=0x060A00 baseline
- [ ] Deprecation warnings treated as errors

## Exception Process

If you must use a deprecated library:
1. **STOP** - Do not proceed with implementation
2. **Create ADR** - Document the exception with full justification
3. **Get Approval** - Require explicit approval from project maintainers
4. **Document** - Add to Known Deviations in project notes
5. **Mitigation Plan** - Define timeline for migration to non-deprecated alternative

## Automated Checks

The following checks will run automatically:
- [ ] Policy enforcement (policy.yml)
- [ ] Deprecated library detection
- [ ] Build artifact scanning
- [ ] Linked library analysis

## Review Guidelines

Reviewers should verify:
- [ ] No deprecated libraries in code or dependencies
- [ ] Policy compliance maintained
- [ ] Performance impact acceptable
- [ ] Security implications considered
- [ ] Documentation complete

---

**Remember:** Policy violations will result in immediate PR rejection. When in doubt, ask for guidance before implementation.




