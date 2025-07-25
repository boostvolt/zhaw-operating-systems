# Lab Markdown Documentation Rule

## Output Verification

- Every shell command that Cursor proposes, explains, or documents—whether as a command, command output, or any explanation based on a command—must be verified by actually running the command on the provided SSH server (ssh ubuntu@160.85.31.224).
- The output, behavior, and any conclusions must be taken directly from the SSH server's response.
- The SSH server is the authoritative source for command correctness, output, and system behavior.
- Cursor must always compare its intended response against the real output from the SSH server and use the server's response as the ground truth.
- No command, output, or explanation should be fabricated or assumed; all must be validated by the actual Ubuntu server. any lab or exercise markdown file **must be verified by running the actual commands on the provided SSH server** (`ssh ubuntu@160.85.31.224`).
- Outputs in the markdown should be real, not fabricated. Use abridged outputs for clarity, but always reflect the real structure and content.
- If a command's output is summarized, indicate this with 'Output (abridged):' and use `...` for omitted lines.

## Documentation Structure

- Preserve the original lab structure and instructions exactly as written in the markdown.
- Add findings, outputs, and explanations as clearly marked additions (e.g., '**[Lab Output & Explanation]**') directly after each relevant bullet or subtask.
- Use markdown code blocks for commands and outputs.
- Use concise, exam-focused explanations and summary tables where appropriate.
- Do not introduce new checklists or headings unless explicitly requested.

## README Linking

- If a lab markdown file is not referenced in the [README.md](mdc:README.md), always add a link to it in the README under a relevant section (e.g., 'Labs' or 'Course Materials').

## General Style

- Use clear, technical language suitable for exam preparation.
- When in doubt, prefer real output and concise summaries over hypothetical or verbose explanations.
