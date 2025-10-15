# Sprint 002 Execution Plan: Chat Features

**Branch:** `sprint-002-chat-features`  
**Status:** 🔄 In Progress (Day 3 of 5)  
**Duration:** January 8-12, 2025 (5 days)

---

## Pre-Flight Checklist

- [x] `.underlord/preflight.sh` passes
- [x] Review `goals.md` and `context.md`
- [x] Confirm branch: `sprint-002-chat-features`
- [x] Working directory clean
- [x] Latest main pulled

---

## Phase 1: Database Schema & Models

**Goal:** Create data models for chat functionality

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [x] Create `models/chat_message.py` with Message model
   - Fields: id, conversation_id, role, content, timestamp, metadata
   - Add SQLAlchemy ORM mappings
   - Include created_at/updated_at timestamps

2. [x] Create `models/conversation.py` with Conversation model
   - Fields: id, user_id, title, created_at, updated_at
   - Add relationship to messages
   - Include session metadata

3. [x] Add Alembic migration for chat tables
   - Create migration script
   - Add indexes for common queries
   - Test migration up/down

4. [x] Write unit tests for models
   - Test model creation
   - Test relationships
   - Test validation

### Verification Checklist:
- [x] Models import without errors
- [x] Migration applies cleanly
- [x] All model tests pass
- [x] Database schema matches design

**Outcome:** ✅ Database schema implemented

**Actual Time:** 4 hours (estimated: 4 hours)

---

## Phase 2: Chat API Endpoint

**Goal:** Implement POST /api/chat endpoint

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [x] Create `routers/chat.py` with POST /api/chat endpoint
   - Accept user message in request body
   - Validate input (max length, required fields)
   - Return chat response

2. [x] Add request/response schemas
   - ChatRequest: message, conversation_id (optional)
   - ChatResponse: message, conversation_id, timestamp
   - Include OpenAPI examples

3. [x] Implement basic endpoint logic
   - Save user message to database
   - Generate mock response (placeholder)
   - Return formatted response

4. [x] Write endpoint tests
   - Test successful chat
   - Test validation errors
   - Test database persistence

5. [x] Update OpenAPI documentation
   - Add endpoint description
   - Include request/response examples
   - Document error codes

### Verification Checklist:
- [x] Endpoint responds correctly
- [x] Request validation works
- [x] Messages saved to database
- [x] All tests pass

**Outcome:** ✅ Chat endpoint operational

**Actual Time:** 5 hours (estimated: 6 hours)

---

## Phase 3: Bedrock Model Integration

**Goal:** Connect chat endpoint to AWS Bedrock for real responses

**Breakpoint:** 🛑 Human review after integration

### Implementation Steps:

1. [x] Create `services/chat_service.py`
   - Implement chat logic
   - Handle conversation context
   - Format prompts for Bedrock

2. [🔄] Integrate Bedrock model invocation
   - Add retry logic with exponential backoff
   - Handle rate limiting (429 errors)
   - Add timeout handling (120s)
   - Log all API calls

3. [🔄] Implement conversation history management
   - Retrieve last N messages from database
   - Format history for context window
   - Limit to 4000 tokens
   - Handle token counting

4. [ ] Add error handling
   - Catch Bedrock API errors
   - Provide fallback responses
   - Log errors for monitoring

5. [ ] Write integration tests
   - Mock Bedrock responses
   - Test with conversation history
   - Test error scenarios

### Verification Checklist:
- [x] Bedrock integration works
- [🔄] Retry logic activates on errors
- [🔄] Conversation history included
- [ ] Error handling comprehensive
- [ ] Integration tests pass

**Outcome:** 🔄 In Progress

**Current Status:** Implementing retry logic and history management

**Estimated Completion:** End of Day 3

---

## Phase 4: Streaming Response Support

**Goal:** Enable real-time streaming of chat responses

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [ ] Modify endpoint to support Server-Sent Events (SSE)
   - Add streaming endpoint: POST /api/chat/stream
   - Implement SSE response format
   - Handle connection lifecycle

2. [ ] Implement streaming in chat service
   - Use Bedrock streaming API
   - Yield response chunks
   - Handle partial responses

3. [ ] Add client-side example in docs
   - JavaScript EventSource example
   - Python streaming client
   - Error handling guide

4. [ ] Write streaming tests
   - Test complete stream
   - Test interrupted stream
   - Test error during stream

### Verification Checklist:
- [ ] Streaming endpoint works
- [ ] Chunks arrive in real-time
- [ ] Connection closes properly
- [ ] Tests pass

**Outcome:** ⏳ Pending

**Estimated Time:** 6 hours

---

## Phase 5: Rate Limiting

**Goal:** Protect API from abuse

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [ ] Install slowapi for rate limiting
   - Add to requirements.txt
   - Configure Redis backend

2. [ ] Apply rate limits to chat endpoints
   - 10 requests per minute per user
   - 100 requests per hour per user
   - 1000 requests per day per IP

3. [ ] Add rate limit headers
   - X-RateLimit-Limit
   - X-RateLimit-Remaining
   - X-RateLimit-Reset

4. [ ] Document rate limits
   - Add to API docs
   - Create error response examples
   - Document retry strategies

5. [ ] Test rate limiting
   - Verify limits enforced
   - Check header values
   - Test limit reset

### Verification Checklist:
- [ ] Rate limits enforced
- [ ] Headers present
- [ ] Documentation updated
- [ ] Tests pass

**Outcome:** ⏳ Pending

**Estimated Time:** 4 hours

---

## Phase 6: Message Validation

**Goal:** Ensure message quality and safety

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [ ] Create validation middleware
   - Check message length (max 10,000 chars)
   - Detect empty messages
   - Sanitize HTML/scripts

2. [ ] Add content safety checks
   - Detect inappropriate content
   - Block malicious inputs
   - Log suspicious activity

3. [ ] Implement input sanitization
   - Remove dangerous characters
   - Normalize whitespace
   - Validate encoding

4. [ ] Write validation tests
   - Test valid messages
   - Test invalid messages
   - Test edge cases

### Verification Checklist:
- [ ] Validation catches issues
- [ ] Sanitization preserves intent
- [ ] All tests pass
- [ ] Security verified

**Outcome:** ⏳ Pending

**Estimated Time:** 5 hours

---

## Phase 7: Integration Tests

**Goal:** Verify end-to-end chat flow

**Breakpoint:** 🛑 Human review after tests pass

### Implementation Steps:

1. [ ] Create integration test suite
   - Test complete chat flow
   - Test with real database
   - Mock Bedrock API

2. [ ] Test conversation continuity
   - Create conversation
   - Send multiple messages
   - Verify history maintained

3. [ ] Test error scenarios
   - Database connection failures
   - Bedrock API failures
   - Invalid inputs

4. [ ] Add performance tests
   - Measure response times
   - Test concurrent requests
   - Check memory usage

### Verification Checklist:
- [ ] All integration tests pass
- [ ] Performance acceptable (<2s p95)
- [ ] No memory leaks
- [ ] Error handling robust

**Outcome:** ⏳ Pending

**Estimated Time:** 6 hours

---

## Phase 8: Monitoring & Logging

**Goal:** Enable observability of chat system

**Breakpoint:** 🛑 Human review after implementation

### Implementation Steps:

1. [ ] Add structured logging
   - Log all chat requests
   - Log Bedrock API calls
   - Log errors with context

2. [ ] Create metrics
   - Chat requests per minute
   - Average response time
   - Error rate

3. [ ] Set up alerts
   - High error rate
   - Slow responses
   - Rate limit violations

4. [ ] Document monitoring
   - Add runbook for common issues
   - Document metric meanings
   - Create dashboard guide

### Verification Checklist:
- [ ] Logs are structured
- [ ] Metrics collecting
- [ ] Alerts configured
- [ ] Documentation complete

**Outcome:** ⏳ Pending

**Estimated Time:** 4 hours

---

## Phase 9: API Documentation

**Goal:** Complete chat API documentation

**Breakpoint:** 🛑 Human review before PR

### Implementation Steps:

1. [ ] Update OpenAPI specs
   - Add all chat endpoints
   - Include examples
   - Document error codes

2. [ ] Write integration guide
   - Quick start example
   - Best practices
   - Common patterns

3. [ ] Add code samples
   - Python client
   - JavaScript client
   - cURL examples

4. [ ] Create troubleshooting guide
   - Common errors
   - Debug strategies
   - Support resources

### Verification Checklist:
- [ ] All endpoints documented
- [ ] Examples work
- [ ] Guide is clear
- [ ] Links functional

**Outcome:** ⏳ Pending

**Estimated Time:** 3 hours

---

## Phase 10: Final Review & Deployment

**Goal:** Complete sprint and deploy to staging

**Breakpoint:** 🛑 Human approval required before merge

### Final Steps:

1. [ ] Code review completed
2. [ ] All tests passing (unit + integration)
3. [ ] Coverage >85%
4. [ ] Documentation reviewed
5. [ ] Deploy to staging
6. [ ] Smoke tests pass
7. [ ] Performance acceptable
8. [ ] Security scan clean

### Verification Checklist:
- [ ] All PR comments addressed
- [ ] CI/CD green
- [ ] Staging deployment successful
- [ ] Ready for production

**Outcome:** ⏳ Pending

---

## Current Progress Summary

**Completed:** 4 tasks (Phase 1-2 complete)  
**In Progress:** 2 tasks (Phase 3)  
**Pending:** 4 tasks (Phase 4-10)  
**Progress:** 40% complete

**On Track:** Yes (Day 3 of 5)

---

## Emergency Procedures

### If Build Fails Unexpectedly
```bash
rm -rf build
.underlord/preflight.sh
```

### If Bedrock Integration Fails
1. Check AWS credentials
2. Verify model ID
3. Check service quotas
4. Review CloudWatch logs

### If Tests Fail
1. Run individual test: `pytest tests/test_specific.py -v`
2. Check test logs
3. Verify test database clean

---

## Notes

**Day 1-2 (Completed):**
- Database schema cleaner than expected
- Chat endpoint straightforward
- Good test coverage (82%)

**Day 3 (Current):**
- Bedrock retry logic more complex than planned
- Conversation history token counting needs attention
- May need to adjust Phase 4 timeline

**Risks:**
- Streaming implementation could be complex (Phase 4)
- Rate limiting Redis dependency (Phase 5)
- Integration tests may reveal edge cases (Phase 7)

---

**Last Updated:** 2025-01-10 (Day 3)  
**Next Update:** 2025-01-11 (End of Day 4)
