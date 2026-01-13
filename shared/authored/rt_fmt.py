#!/usr/bin/env python3
import sys
import re

# --- RT Code Formatting Rules ---
# [Rules 1-6 unchanged]
# 7. Vertical Lists:
#    - Trailing commas (end of line) are migrated to the start of the next line.
#    - Vertical commas get a space before them (`,item`).

# --- Globals ---
RT_DEBUG = True
MAX_LOOPS = 100

# --- Data Structures ---
class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value
    
    def __repr__(self):
        return f"<{self.type}:'{repr(self.value)}'>"

# Types
T_NEWLINE = 'NEWLINE'
T_INDENT  = 'INDENT'
T_BLANK   = 'BLANK'
T_OPEN    = 'OPEN'   # ( { [
T_CLOSE   = 'CLOSE'  # ) } ]
T_COMMA   = 'COMMA'  # ,
T_STUFF   = 'STUFF'  # Code

# --- Tokenizer ---
def tokenize(text):
    tokens = []
    placeholders = {}
    def mask_match(match):
        key = f"__RT_MASK_{len(placeholders)}__"
        placeholders[key] = match.group(0)
        return key

    masked_text = re.sub(r'("|l\').*?(\1)|//.*|#.*', mask_match, text, flags=re.MULTILINE)

    i = 0
    length = len(masked_text)
    at_line_start = True
    
    while i < length:
        char = masked_text[i]
        
        if char == '\n':
            tokens.append(Token(T_NEWLINE, char))
            at_line_start = True
            i += 1
            continue
            
        if char.isspace():
            start = i
            while i < length and masked_text[i].isspace() and masked_text[i] != '\n':
                i += 1
            val = masked_text[start:i]
            if at_line_start:
                tokens.append(Token(T_INDENT, val))
            else:
                tokens.append(Token(T_BLANK, val))
            continue

        at_line_start = False
        
        if char in '({[':
            tokens.append(Token(T_OPEN, char))
            i += 1
        elif char in ')}]':
            tokens.append(Token(T_CLOSE, char))
            i += 1
        elif char == ',':
            tokens.append(Token(T_COMMA, char))
            i += 1
        else:
            start = i
            while i < length:
                c = masked_text[i]
                if c.isspace() or c in '({[]}),\n':
                    break
                i += 1
            val = masked_text[start:i]
            for k,v in placeholders.items():
                if k in val: val = val.replace(k,v)
            tokens.append(Token(T_STUFF, val))

    return tokens

def detokenize(tokens):
    return "".join(t.value for t in tokens)

# --- Transformers (Actions) ---
def fmt_tok(t):
    return f"{t.type}('{t.value}')"

def log_action(name, idx, tokens):
    if RT_DEBUG:
        prev_s = fmt_tok(tokens[idx-1]) if idx > 0 else "START"
        curr_s = fmt_tok(tokens[idx])
        next_s = fmt_tok(tokens[idx+1]) if idx+1 < len(tokens) else "END"
        print(f"[ACTION] {name} at {idx}: {prev_s} << {curr_s} >> {next_s}")

def action_remove_next(tokens, idx):
    if idx + 1 < len(tokens):
        log_action("REMOVE NEXT", idx, tokens)
        del tokens[idx+1]
        return True
    return False

def action_remove_prev(tokens, idx):
    if idx > 0:
        log_action("REMOVE PREV", idx, tokens)
        del tokens[idx-1]
        return True
    return False

def action_insert_space_next(tokens, idx):
    log_action("INSERT SPACE NEXT", idx, tokens)
    tokens.insert(idx + 1, Token(T_BLANK, " "))
    return True

def action_insert_space_prev(tokens, idx):
    log_action("INSERT SPACE PREV", idx, tokens)
    tokens.insert(idx, Token(T_BLANK, " "))
    return True

def action_fix_excess_space_next(tokens, idx):
    if idx + 1 < len(tokens) and tokens[idx+1].type == T_BLANK:
        if tokens[idx+1].value != " ":
            log_action("FIX EXCESS SPACE NEXT", idx, tokens)
            tokens[idx+1].value = " "
            return True
    return False

def action_fix_excess_space_prev(tokens, idx):
    if idx > 0 and tokens[idx-1].type == T_BLANK:
        if tokens[idx-1].value != " ":
            log_action("FIX EXCESS SPACE PREV", idx, tokens)
            tokens[idx-1].value = " "
            return True
    return False

# --- Recognizers (Predicates) ---
def is_open(t): return t.type == T_OPEN
def is_close(t): return t.type == T_CLOSE
def is_blank(t): return t.type == T_BLANK
def is_stuff(t): return t.type == T_STUFF
def is_comma(t): return t.type == T_COMMA

def log_pred(name, idx, result, reason=""):
    if RT_DEBUG:
        r_str = "YES" if result else "NO "
        print(f"[PRED] {name}({idx}) -> {r_str} ({reason})")

def analyze_line_structure(tokens, idx):
    line_start = idx
    while line_start > 0 and tokens[line_start].type != T_NEWLINE:
        line_start -= 1
    if tokens[line_start].type == T_NEWLINE: line_start += 1 
    
    line_end = idx
    while line_end < len(tokens) and tokens[line_end].type != T_NEWLINE:
        line_end += 1
        
    stack = []
    pairs = {}   
    orphans = [] 
    
    for i in range(line_start, line_end):
        t = tokens[i]
        if is_open(t):
            stack.append(i)
        elif is_close(t):
            if stack:
                s = stack.pop()
                pairs[s] = i
            else:
                orphans.append(i)
    widows = stack 
    return pairs, widows, orphans, line_start, line_end

def is_perimeter_start(tokens, idx):
    if not is_open(tokens[idx]): return False
    pairs, widows, orphans, l_start, l_end = analyze_line_structure(tokens, idx)
    if idx in pairs:
        closer_idx = pairs[idx]
        for s, e in pairs.items():
            if s < idx and e > closer_idx: return False 
        has_structure = False
        for k in range(idx + 1, closer_idx):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        return has_structure
    elif idx in widows:
        if widows[0] != idx: return False 
        has_structure = False
        for k in range(idx + 1, l_end):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        if has_structure:
            log_pred("is_perimeter_start", idx, True, "First Widow with Structure")
            return True
    return False

def is_perimeter_end(tokens, idx):
    if not is_close(tokens[idx]): return False
    pairs, widows, orphans, l_start, l_end = analyze_line_structure(tokens, idx)
    reverse_pairs = {v: k for k, v in pairs.items()}
    if idx in reverse_pairs:
        opener_idx = reverse_pairs[idx]
        for s, e in pairs.items():
            if s < opener_idx and e > idx: return False
        has_structure = False
        for k in range(opener_idx + 1, idx):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        return has_structure
    elif idx in orphans:
        if orphans[-1] != idx: return False
        return True 
    return False

def has_exact_following_space(tokens, idx):
    if idx + 1 < len(tokens) and is_blank(tokens[idx+1]):
        return tokens[idx+1].value == " "
    return False

def has_exact_preceding_space(tokens, idx):
    if idx > 0 and is_blank(tokens[idx-1]):
        return tokens[idx-1].value == " "
    return False

def has_any_following_space(tokens, idx):
    return idx + 1 < len(tokens) and is_blank(tokens[idx+1])

def has_any_preceding_space(tokens, idx):
    return idx > 0 and is_blank(tokens[idx-1])

def is_adjacent_closer(tokens, idx):
    if not is_close(tokens[idx]): return False
    if idx > 0 and is_close(tokens[idx-1]): return True
    return False

def is_brace_after_paren(tokens, idx):
    if tokens[idx].value != '{': return False
    prev_idx = idx - 1
    if prev_idx < 0: return False
    if is_blank(tokens[prev_idx]): prev_idx -= 1
    if prev_idx >= 0 and tokens[prev_idx].value == ')': return True
    return False

def is_identifier(token):
    if token.type != T_STUFF or not token.value: return False
    c = token.value[0]
    return c.isalpha() or c == '_'

# --- Enclosure Fixers ---
def fix_perimeter_open(tokens, idx):
    if is_perimeter_start(tokens, idx):
        if not has_any_following_space(tokens, idx):
            return action_insert_space_next(tokens, idx)
        elif not has_exact_following_space(tokens, idx):
            return action_fix_excess_space_next(tokens, idx)
    return False

def fix_perimeter_close(tokens, idx):
    if is_perimeter_end(tokens, idx):
        if not has_any_preceding_space(tokens, idx):
            return action_insert_space_prev(tokens, idx)
        elif not has_exact_preceding_space(tokens, idx):
            return action_fix_excess_space_prev(tokens, idx)
    return False

def fix_interior_open(tokens, idx):
    if is_open(tokens[idx]) and not is_perimeter_start(tokens, idx):
        if has_any_following_space(tokens, idx):
            return action_remove_next(tokens, idx)
    return False

def fix_interior_close(tokens, idx):
    if is_close(tokens[idx]) and not is_perimeter_end(tokens, idx):
        if has_any_preceding_space(tokens, idx):
            prev_idx = idx - 1
            if prev_idx >= 0 and tokens[prev_idx].type == T_INDENT: return False
            if idx > 1 and is_close(tokens[idx-2]): return False
            return action_remove_prev(tokens, idx)
    return False

def fix_bookend_stack(tokens, idx):
    if is_adjacent_closer(tokens, idx):
        if not has_any_preceding_space(tokens, idx):
            return action_insert_space_prev(tokens, idx)
    return False

def fix_brace_after_paren(tokens, idx):
    if is_brace_after_paren(tokens, idx):
        if has_any_preceding_space(tokens, idx):
            return action_remove_prev(tokens, idx)
    return False
    
def fix_close_paren_followed_by_alpha(tokens, idx):
    if is_close(tokens[idx]) and idx+1 < len(tokens):
        nxt = tokens[idx+1]
        if is_identifier(nxt):
            if not has_any_following_space(tokens, idx):
                return action_insert_space_next(tokens, idx)
    return False

# --- Phase 1: Enclosure Loop ---
def format_enclosure_structure(tokens):
    fixers = [
        fix_bookend_stack,
        fix_brace_after_paren,
        fix_perimeter_open,
        fix_perimeter_close,
        fix_interior_open,
        fix_interior_close,
        fix_close_paren_followed_by_alpha
    ]
    i = 0
    while i < len(tokens):
        stable = False
        loops = 0
        while not stable:
            if loops > MAX_LOOPS: break
            loops += 1
            action_taken = False
            for func in fixers:
                if func(tokens, i):
                    action_taken = True
                    break 
            if not action_taken: stable = True
        i += 1
    return tokens

# --- Phase 2: Trailing Comma Migration ---
def pass_migrate_trailing_commas(tokens):
    """
    Moves trailing commas to the start of the next line.
    Rule: ITEM, [BLANK] NEWLINE [INDENT] [BLANK] ITEM -> ITEM NEWLINE [INDENT] [BLANK] , ITEM
    Also cleans up blank lines created by removing the comma.
    """
    i = 0
    while i < len(tokens):
        t = tokens[i]
        
        if t.type == T_COMMA:
            # 1. Check Context: Is this a trailing comma?
            # Look ahead for NEWLINE (skipping blanks)
            j = i + 1
            while j < len(tokens) and tokens[j].type == T_BLANK:
                j += 1
            
            if j < len(tokens) and tokens[j].type == T_NEWLINE:
                if RT_DEBUG: print(f"[MIGRATE] Found trailing comma at {i}")
                
                # --- Step A: Remove Comma from Current Line ---
                # Remove range [i, j) (The comma and trailing spaces)
                del tokens[i:j]
                
                # Note: tokens[i] is now the NEWLINE.
                current_newline_idx = i
                
                # --- Step B: Cleanup Empty Line ---
                # Check if the line we just stripped is now empty (only whitespace/indent).
                # Scan backwards from NEWLINE to find start of line.
                start_of_line_idx = current_newline_idx
                while start_of_line_idx > 0 and tokens[start_of_line_idx-1].type != T_NEWLINE:
                    start_of_line_idx -= 1
                
                # Check contents
                is_empty_line = True
                for k in range(start_of_line_idx, current_newline_idx):
                    if tokens[k].type not in (T_INDENT, T_BLANK):
                        is_empty_line = False
                        break
                
                if is_empty_line and start_of_line_idx < current_newline_idx:
                     if RT_DEBUG: print("[MIGRATE] Collapsing empty line")
                     # Delete the line content + the NEWLINE at the end.
                     # Range: [start_of_line_idx, current_newline_idx + 1]
                     del tokens[start_of_line_idx : current_newline_idx + 1]
                     
                     # We need to find the *new* current_newline_idx because indices shifted.
                     # We are now sitting at 'start_of_line_idx', which is effectively the start of the next line.
                     # But we need to insert the comma AFTER the newline of the *previous* line.
                     # Wait, if we collapsed the line, we effectively merged the previous item up?
                     # No, typically migration happens in a list:
                     # Item A, \n Item B
                     # Remove comma -> Item A \n Item B.
                     # If Item A line was empty? " , \n" -> Remove comma -> " \n".
                     # Then remove line -> Item B moves up.
                     # The comma should go before Item B.
                     
                     # Correct target: The insertion point is at `start_of_line_idx` (which is now the start of next line).
                     # We want to insert ", " there?
                     # If we collapsed, we might have lost the newline separator entirely?
                     # Let's assume standard trailing comma case first:
                     # "Item A, \n" -> "Item A \n , Item B"
                     pass 
                
                # --- Step C: Locate Insertion Point on Next Line ---
                # We need to find the NEWLINE token to insert AFTER.
                # If we collapsed the line, 'i' might point to start of next line (INDENT/STUFF).
                # If we didn't collapse, 'tokens[i]' IS the NEWLINE.
                
                # Let's recover the insertion point relative to 'i'.
                # We want to scan past NEWLINE, INDENT, BLANK on the *current* stream state.
                
                # If we didn't collapse, tokens[i] is NEWLINE.
                # If we collapsed, we deleted the newline. We need to find where to put the comma.
                # If we collapsed " , \n", the comma belongs to the item *after* the deleted line.
                # So we insert at 'i'.
                
                insert_pos = i
                
                if not is_empty_line:
                    # We are at NEWLINE. Skip it.
                    if insert_pos < len(tokens) and tokens[insert_pos].type == T_NEWLINE:
                        insert_pos += 1
                
                # Skip Indent and Blank on the destination line
                while insert_pos < len(tokens) and tokens[insert_pos].type in (T_INDENT, T_BLANK):
                    insert_pos += 1
                    
                # --- Step D: Insert Comma ---
                if RT_DEBUG: print(f"[MIGRATE] Inserting comma at {insert_pos}")
                
                # Insert Space + Comma (Vertical comma style: " ,Item")
                # Wait, "Vertical commas get a space before them".
                # So we insert " " then ",".
                tokens.insert(insert_pos, Token(T_COMMA, ","))
                tokens.insert(insert_pos, Token(T_BLANK, " "))
                
                # Continue scan from after insertion
                i = insert_pos + 2 
                continue

        i += 1
    return tokens

# --- Phase 3: Horizontal Comma Pass ---
def format_horizontal_commas(tokens):
    i = 0
    while i < len(tokens):
        t = tokens[i]
        if t.type == T_COMMA:
            is_vertical = False
            # Check if prev is NEWLINE or INDENT (implies vertical)
            # OR if prev is BLANK and prev-prev is NEWLINE/INDENT?
            # Migration pass ensures " \n [INDENT] [BLANK] ,"
            # So looking back 1 or 2 tokens is sufficient.
            
            lookback = i - 1
            while lookback >= 0 and tokens[lookback].type == T_BLANK:
                lookback -= 1
            
            if lookback >= 0 and tokens[lookback].type in (T_NEWLINE, T_INDENT):
                is_vertical = True
            
            if not is_vertical:
                if not has_any_preceding_space(tokens, i):
                    action_insert_space_prev(tokens, i)
                    i += 1 
                if has_any_following_space(tokens, i):
                    action_remove_next(tokens, i)
        i += 1
    return tokens

# --- Driver ---
def process_file(in_fp, out_fp):
    with open(in_fp, 'r') as f:
        content = f.read()
    
    tokens = tokenize(content)

    if RT_DEBUG:
        print("--- DEBUG: Token Stream (Initial) ---")
        for idx, t in enumerate(tokens): print(f"{idx}: {t}")

    # Phase 1: Walls
    tokens = format_enclosure_structure(tokens)
    
    # Phase 2: Migration
    tokens = pass_migrate_trailing_commas(tokens)
    
    # Phase 3: Furniture
    tokens = format_horizontal_commas(tokens)
    
    with open(out_fp, 'w') as f:
        f.write(detokenize(tokens))

def CLI():
    global RT_DEBUG
    args = sys.argv[1:]
    if "-d" in args:
        RT_DEBUG = True
        args.remove("-d")
    if len(args) < 1:
        print("Usage: rt_fmt [-d] <in_file> [out_file]")
        sys.exit(1)
    in_fp = args[0]
    out_fp = args[1] if len(args) > 1 else in_fp
    process_file(in_fp, out_fp)
    if RT_DEBUG: print(f"Formatted: {in_fp} -> {out_fp}")

if __name__ == "__main__": CLI()




#!/usr/bin/env python3
import sys
import re

# --- RT Code Formatting Rules ---
# [Rules 1-6 unchanged...]

# --- Globals ---
RT_DEBUG = False

# --- Data Structures ---
class Token:
    def __init__(self, type, value):
        self.type = type
        self.value = value
    
    def __repr__(self):
        return f"<{self.type}:'{repr(self.value)}'>"

# Types
T_NEWLINE = 'NEWLINE'
T_INDENT  = 'INDENT'
T_BLANK   = 'BLANK'
T_OPEN    = 'OPEN'   # ( { [
T_CLOSE   = 'CLOSE'  # ) } ]
T_COMMA   = 'COMMA'  # ,
T_STUFF   = 'STUFF'  # Code

# --- Tokenizer ---
def tokenize(text):
    tokens = []
    placeholders = {}
    def mask_match(match):
        key = f"__RT_MASK_{len(placeholders)}__"
        placeholders[key] = match.group(0)
        return key

    masked_text = re.sub(r'("|l\').*?(\1)|//.*|#.*', mask_match, text, flags=re.MULTILINE)

    i = 0
    length = len(masked_text)
    at_line_start = True
    
    while i < length:
        char = masked_text[i]
        
        if char == '\n':
            tokens.append(Token(T_NEWLINE, char))
            at_line_start = True
            i += 1
            continue
            
        if char.isspace():
            start = i
            while i < length and masked_text[i].isspace() and masked_text[i] != '\n':
                i += 1
            val = masked_text[start:i]
            if at_line_start:
                tokens.append(Token(T_INDENT, val))
            else:
                tokens.append(Token(T_BLANK, val))
            continue

        at_line_start = False
        
        if char in '({[':
            tokens.append(Token(T_OPEN, char))
            i += 1
        elif char in ')}]':
            tokens.append(Token(T_CLOSE, char))
            i += 1
        elif char == ',':
            tokens.append(Token(T_COMMA, char))
            i += 1
        else:
            start = i
            while i < length:
                c = masked_text[i]
                if c.isspace() or c in '({[]}),\n':
                    break
                i += 1
            val = masked_text[start:i]
            for k,v in placeholders.items():
                if k in val: val = val.replace(k,v)
            tokens.append(Token(T_STUFF, val))

    return tokens

def detokenize(tokens):
    return "".join(t.value for t in tokens)

# --- Transformers (Actions) ---
def fmt_tok(t):
    return f"{t.type}('{t.value}')"

def log_action(name, idx, tokens):
    if RT_DEBUG:
        prev_s = fmt_tok(tokens[idx-1]) if idx > 0 else "START"
        curr_s = fmt_tok(tokens[idx])
        next_s = fmt_tok(tokens[idx+1]) if idx+1 < len(tokens) else "END"
        print(f"[ACTION] {name} at {idx}: {prev_s} << {curr_s} >> {next_s}")

def action_remove_next(tokens, idx):
    if idx + 1 < len(tokens):
        log_action("REMOVE NEXT", idx, tokens)
        del tokens[idx+1]
        return True
    return False

def action_remove_prev(tokens, idx):
    if idx > 0:
        log_action("REMOVE PREV", idx, tokens)
        del tokens[idx-1]
        return True
    return False

def action_insert_space_next(tokens, idx):
    log_action("INSERT SPACE NEXT", idx, tokens)
    tokens.insert(idx + 1, Token(T_BLANK, " "))
    return True

def action_insert_space_prev(tokens, idx):
    log_action("INSERT SPACE PREV", idx, tokens)
    tokens.insert(idx, Token(T_BLANK, " "))
    return True

def action_fix_excess_space_next(tokens, idx):
    if idx + 1 < len(tokens) and tokens[idx+1].type == T_BLANK:
        if tokens[idx+1].value != " ":
            log_action("FIX EXCESS SPACE NEXT", idx, tokens)
            tokens[idx+1].value = " "
            return True
    return False

def action_fix_excess_space_prev(tokens, idx):
    if idx > 0 and tokens[idx-1].type == T_BLANK:
        if tokens[idx-1].value != " ":
            log_action("FIX EXCESS SPACE PREV", idx, tokens)
            tokens[idx-1].value = " "
            return True
    return False

# --- Recognizers (Predicates) ---
def is_open(t): return t.type == T_OPEN
def is_close(t): return t.type == T_CLOSE
def is_blank(t): return t.type == T_BLANK
def is_stuff(t): return t.type == T_STUFF
def is_comma(t): return t.type == T_COMMA

def log_pred(name, idx, result, reason=""):
    if RT_DEBUG:
        r_str = "YES" if result else "NO "
        print(f"[PRED] {name}({idx}) -> {r_str} ({reason})")

def analyze_line_structure(tokens, idx):
    line_start = idx
    while line_start > 0 and tokens[line_start].type != T_NEWLINE:
        line_start -= 1
    if tokens[line_start].type == T_NEWLINE: line_start += 1 
    
    line_end = idx
    while line_end < len(tokens) and tokens[line_end].type != T_NEWLINE:
        line_end += 1
        
    stack = []
    pairs = {}   
    orphans = [] 
    
    for i in range(line_start, line_end):
        t = tokens[i]
        if is_open(t):
            stack.append(i)
        elif is_close(t):
            if stack:
                s = stack.pop()
                pairs[s] = i
            else:
                orphans.append(i)
    widows = stack 
    return pairs, widows, orphans, line_start, line_end

def is_perimeter_start(tokens, idx):
    if not is_open(tokens[idx]): return False
    pairs, widows, orphans, l_start, l_end = analyze_line_structure(tokens, idx)
    if idx in pairs:
        closer_idx = pairs[idx]
        for s, e in pairs.items():
            if s < idx and e > closer_idx: return False 
        has_structure = False
        for k in range(idx + 1, closer_idx):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        return has_structure
    elif idx in widows:
        if widows[0] != idx: return False 
        has_structure = False
        for k in range(idx + 1, l_end):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        if has_structure:
            log_pred("is_perimeter_start", idx, True, "First Widow with Structure")
            return True
    return False

def is_perimeter_end(tokens, idx):
    if not is_close(tokens[idx]): return False
    pairs, widows, orphans, l_start, l_end = analyze_line_structure(tokens, idx)
    reverse_pairs = {v: k for k, v in pairs.items()}
    if idx in reverse_pairs:
        opener_idx = reverse_pairs[idx]
        for s, e in pairs.items():
            if s < opener_idx and e > idx: return False
        has_structure = False
        for k in range(opener_idx + 1, idx):
            if tokens[k].type in (T_OPEN, T_CLOSE):
                has_structure = True
                break
        return has_structure
    elif idx in orphans:
        if orphans[-1] != idx: return False
        return True 
    return False

def has_exact_following_space(tokens, idx):
    if idx + 1 < len(tokens) and is_blank(tokens[idx+1]):
        return tokens[idx+1].value == " "
    return False

def has_exact_preceding_space(tokens, idx):
    if idx > 0 and is_blank(tokens[idx-1]):
        return tokens[idx-1].value == " "
    return False

def has_any_following_space(tokens, idx):
    return idx + 1 < len(tokens) and is_blank(tokens[idx+1])

def has_any_preceding_space(tokens, idx):
    return idx > 0 and is_blank(tokens[idx-1])

def is_adjacent_closer(tokens, idx):
    if not is_close(tokens[idx]): return False
    if idx > 0 and is_close(tokens[idx-1]): return True
    return False

def is_brace_after_paren(tokens, idx):
    if tokens[idx].value != '{': return False
    prev_idx = idx - 1
    if prev_idx < 0: return False
    if is_blank(tokens[prev_idx]): prev_idx -= 1
    if prev_idx >= 0 and tokens[prev_idx].value == ')': return True
    return False

def is_identifier(token):
    if token.type != T_STUFF or not token.value: return False
    c = token.value[0]
    return c.isalpha() or c == '_'

# --- Enclosure Fixers ---
def fix_perimeter_open(tokens, idx):
    if is_perimeter_start(tokens, idx):
        if not has_any_following_space(tokens, idx):
            return action_insert_space_next(tokens, idx)
        elif not has_exact_following_space(tokens, idx):
            return action_fix_excess_space_next(tokens, idx)
    return False

def fix_perimeter_close(tokens, idx):
    if is_perimeter_end(tokens, idx):
        if not has_any_preceding_space(tokens, idx):
            return action_insert_space_prev(tokens, idx)
        elif not has_exact_preceding_space(tokens, idx):
            return action_fix_excess_space_prev(tokens, idx)
    return False

def fix_interior_open(tokens, idx):
    if is_open(tokens[idx]) and not is_perimeter_start(tokens, idx):
        if has_any_following_space(tokens, idx):
            return action_remove_next(tokens, idx)
    return False

def fix_interior_close(tokens, idx):
    if is_close(tokens[idx]) and not is_perimeter_end(tokens, idx):
        if has_any_preceding_space(tokens, idx):
            prev_idx = idx - 1
            if prev_idx >= 0 and tokens[prev_idx].type == T_INDENT: return False
            if idx > 1 and is_close(tokens[idx-2]): return False
            return action_remove_prev(tokens, idx)
    return False

def fix_bookend_stack(tokens, idx):
    if is_adjacent_closer(tokens, idx):
        if not has_any_preceding_space(tokens, idx):
            return action_insert_space_prev(tokens, idx)
    return False

def fix_brace_after_paren(tokens, idx):
    if is_brace_after_paren(tokens, idx):
        if has_any_preceding_space(tokens, idx):
            return action_remove_prev(tokens, idx)
    return False
    
def fix_close_paren_followed_by_alpha(tokens, idx):
    if is_close(tokens[idx]) and idx+1 < len(tokens):
        nxt = tokens[idx+1]
        if is_identifier(nxt):
            if not has_any_following_space(tokens, idx):
                return action_insert_space_next(tokens, idx)
    return False

# --- Phase 1: Enclosure Loop ---
def format_enclosure_structure(tokens):
    fixers = [
        fix_bookend_stack,
        fix_brace_after_paren,
        fix_perimeter_open,
        fix_perimeter_close,
        fix_interior_open,
        fix_interior_close,
        fix_close_paren_followed_by_alpha
    ]
    i = 0
    while i < len(tokens):
        stable = False
        while not stable:
            action_taken = False
            for func in fixers:
                if func(tokens, i):
                    action_taken = True
                    break 
            if not action_taken: stable = True
        i += 1
    return tokens

# --- Phase 2: Horizontal Comma Pass ---
def format_horizontal_commas(tokens):
    """
    Sequential pass for commas.
    Rule: <anything>,<anything> -> <non-space> <space>,<non-space>
    """
    i = 0
    while i < len(tokens):
        t = tokens[i]
        
        # Identify Horizontal Comma
        if t.type == T_COMMA:
            is_vertical = False
            if i > 0 and tokens[i-1].type in (T_NEWLINE, T_INDENT):
                is_vertical = True
            
            if not is_vertical:
                action_taken = False
                
                # 1. Ensure Space BEFORE
                # If prev is NOT blank, insert one.
                # If prev IS blank, ensure it is exactly " " (optional, but good for cleanliness)
                if not has_any_preceding_space(tokens, i):
                    action_insert_space_prev(tokens, i)
                    i += 1 # Adjust index for inserted space
                    action_taken = True
                
                # 2. Ensure NO Space AFTER
                # If next is blank, remove it.
                if has_any_following_space(tokens, i):
                    action_remove_next(tokens, i)
                    # No index adjustment needed (list shrinks, next iter checks new next)
                    action_taken = True

        i += 1
    return tokens

# --- Driver ---
def process_file(in_fp, out_fp):
    with open(in_fp, 'r') as f:
        content = f.read()
    
    tokens = tokenize(content)

    if RT_DEBUG:
        print("--- DEBUG: Token Stream (Initial) ---")
        for idx, t in enumerate(tokens): print(f"{idx}: {t}")

    # Phase 1: Walls
    tokens = format_enclosure_structure(tokens)
    
    # Phase 2: Furniture
    tokens = format_horizontal_commas(tokens)
    
    with open(out_fp, 'w') as f:
        f.write(detokenize(tokens))

def CLI():
    global RT_DEBUG
    args = sys.argv[1:]
    if "-d" in args:
        RT_DEBUG = True
        args.remove("-d")
    if len(args) < 1:
        print("Usage: rt_fmt [-d] <in_file> [out_file]")
        sys.exit(1)
    in_fp = args[0]
    out_fp = args[1] if len(args) > 1 else in_fp
    process_file(in_fp, out_fp)
    if RT_DEBUG: print(f"Formatted: {in_fp} -> {out_fp}")

if __name__ == "__main__": CLI()
