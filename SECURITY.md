# Security Policy

## Reporting a Vulnerability

If you discover a security vulnerability in cbmp, please report it privately
rather than opening a public issue:

- Use GitHub's **"Report a vulnerability"** button under the repository's
  **Security** tab (Security advisories), or
- Email the maintainer.

Please include a description of the issue, the affected version or commit, and
a proof-of-concept or reproduction steps if possible. I will acknowledge the
report, work on a fix, and coordinate disclosure with you.

## Scope and expectations

cbmp parses untrusted input (BMP files). Treat any file passed to `bopen()` as
potentially hostile. Malformed input is expected to be rejected safely rather
than cause memory corruption; reports of the latter are in scope.
