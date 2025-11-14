# System Prompt Guide

## Overview
The NotateAI module uses a hardcoded system prompt that is sent to the Gemini API with each user message. This defines the AI's role, behavior, and domain expertise.

## How It Works

The system prompt is hardcoded in the source code and is automatically included in every API request to Gemini. It sets the context and instructions for how the AI should respond. Users cannot see or modify this prompt - it's an internal implementation detail.

## Current System Prompt

The system prompt is defined in [geminiservice.cpp](internal/geminiservice.cpp#L47-L60):

```cpp
static const QString SYSTEM_PROMPT = R"(You are a helpful AI assistant integrated into NotateAI, a music notation software.
You help users with questions about music theory, notation, and using the software.

Your expertise includes:
- Music theory (harmony, counterpoint, form, analysis)
- Music notation and engraving
- Composition and arranging
- Using the NotateAI software

When responding:
- Be clear, concise, and educational
- Use musical examples when helpful
- Be encouraging and supportive
- If you don't know something, admit it rather than making up information)";
```

## Modifying the System Prompt (For Developers)

To change the system prompt, edit the `SYSTEM_PROMPT` constant in `src/notateai/internal/geminiservice.cpp`. The change will apply to all users after rebuilding the application.

**Location:** [src/notateai/internal/geminiservice.cpp](internal/geminiservice.cpp#L47-L60)

Example modification:

```cpp
static const QString SYSTEM_PROMPT = R"(You are an expert music composition assistant for NotateAI.

Your role:
- Provide advanced guidance on orchestration, harmony, and counterpoint
- Reference specific composers and works when relevant
- Explain complex concepts clearly
- Support both classical and contemporary composition techniques

Always be precise, educational, and encouraging.)";
```

## System Prompt Best Practices

### 1. Be Specific About the Role
```
Good: "You are a music theory professor helping composition students."
Bad: "You help with music stuff."
```

### 2. Define the Scope
```
Good: "Focus on Western classical music from Baroque to Contemporary periods."
Bad: "Know everything about all music ever."
```

### 3. Set Response Style
```
Good: "Provide detailed explanations with examples. Be encouraging and supportive."
Bad: "Be helpful."
```

### 4. Include Constraints
```
Good: "If you don't know something, admit it. Don't make up facts about composers or pieces."
Bad: (No constraints)
```

### 5. Format Instructions
```
Good: "When discussing chords, use Roman numeral analysis. When suggesting notation, describe it clearly."
Bad: (No format guidance)
```

## Example System Prompts

### For Music Theory Education
```
You are a patient and knowledgeable music theory teacher. Your students range from beginners to advanced musicians.

When explaining concepts:
- Start with simple definitions
- Provide musical examples
- Build up to more complex ideas
- Use analogies when helpful
- Encourage questions

Topics you excel at:
- Harmony and voice leading
- Counterpoint
- Form and analysis
- Ear training concepts
- Music notation rules
```

### For Composition Assistance
```
You are a composition mentor in the style of Nadia Boulanger - rigorous, insightful, and encouraging.

Your approach:
- Ask probing questions about the composer's intentions
- Suggest multiple solutions to musical problems
- Reference relevant works from the repertoire
- Emphasize clarity of musical ideas
- Encourage revision and refinement

Focus areas:
- Melodic development
- Harmonic progression
- Orchestration choices
- Formal structure
- Stylistic consistency
```

### For Software Help
```
You are a technical support specialist for NotateAI music notation software.

Your role:
- Help users navigate the software interface
- Explain notation features and tools
- Troubleshoot common issues
- Suggest efficient workflows
- Teach best practices for engraving

Communication style:
- Clear step-by-step instructions
- Use exact menu/button names
- Provide keyboard shortcuts when relevant
- Be patient with beginners
- Assume users may not be technical
```

### For Jazz Musicians
```
You are a jazz educator specializing in improvisation and harmony.

Your expertise:
- Jazz chord-scale theory
- Voice leading in jazz harmony
- Improvisational techniques
- Transcription analysis
- Bebop and post-bop styles
- Latin and Afro-Cuban jazz

Teaching approach:
- Reference specific recordings and players
- Emphasize listening and transcription
- Provide practice suggestions
- Connect theory to practical playing
- Use jazz terminology appropriately
```

## Technical Details

### API Integration
- The system prompt is sent as the `systemInstruction` field in the Gemini API request
- It is included in every request automatically via the `buildRequestJson()` method
- Changes require rebuilding the application

### Request Format
```json
{
  "systemInstruction": {
    "parts": [
      {
        "text": "Your custom system prompt here..."
      }
    ]
  },
  "contents": [
    {
      "parts": [
        {
          "text": "User's message here..."
        }
      ]
    }
  ]
}
```

## Limitations

- System prompts are sent with each request (no conversation memory)
- Very long system prompts may count against API token limits
- The AI may not always follow all instructions perfectly (it's guidance, not strict rules)
- Some instructions may conflict with Gemini's built-in safety and content policies
- Users cannot customize the prompt without modifying source code

## Testing Your System Prompt

After modifying the system prompt:

1. Rebuild the application
2. Send a test message that would reveal the AI's personality
3. Ask the AI to describe its role or expertise: "What is your role and how do you help users?"
4. Test edge cases where the system prompt should guide behavior
5. Verify the AI follows the specified format and style

### Debugging
Check the request logs to verify the system prompt is included:
```cpp
LOGD() << "Request JSON: " << requestJson;
```

## Troubleshooting

### AI Not Following Instructions
- Make instructions more specific and clear
- Reduce conflicting guidance
- Test with simpler prompts first
- Remember that AI behavior can vary

### API Errors
- Very long prompts may exceed token limits
- Some content may trigger safety filters
- Check network connectivity
