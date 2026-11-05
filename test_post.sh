#!/usr/bin/env bash
# Quick API test (server must be running on port 8080)
curl -i -X POST http://localhost:8080/api/contact \
  -H "Content-Type: application/json" \
  -d '{"name":"Test User","email":"test@example.com","message":"Hello from curl"}'
echo
curl http://localhost:8080/api/contact
echo
