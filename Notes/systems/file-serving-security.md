# Safe Static File Serving — Notes

## 1. Problem

Given a user-supplied request path, safely resolve it within a fixed public directory (root) without allowing access to files outside that directory.

---

## 2. Attack Vectors

### 2.1 Directory Traversal

```
/../../etc/passwd
```

Allows escaping the intended directory using `..`.

### 2.2 Absolute Paths

```
/etc/passwd
```

Can override your root if not handled properly.

### 2.3 Encoded Traversal

```
/%2e%2e/%2e%2e/etc/passwd
```

### 2.4 Symlink Escapes

A safe-looking path may point outside root via symlinks.

---

## 3. Key Concepts

### 3.1 Why Stripping `/` is Needed

* User paths like `/images/logo.png` are absolute in URL context.
* Filesystem treats leading `/` as absolute path.
* If not stripped, it can override your root.

**Conclusion:**
Convert user input into a relative path by removing the leading `/`.

---

### 3.2 Why Joining Must Be Careful

* Filesystem join is not string concatenation.
* If the second path is absolute, it **replaces the base path**.

Example:

```
root / /etc/passwd → /etc/passwd ❌
```

**Conclusion:**
Ensure the user path is always relative before joining.

---

### 3.3 Why `..` is Dangerous

* `..` moves to parent directories.
* Can escape the intended root directory.

Example:

```
/public/../../etc/passwd → /etc/passwd ❌
```

**Conclusion:**
Must normalize/remove `..` effects before trusting the path.

---

### 3.4 Why Symlinks Matter

* Symlinks redirect paths to other locations.

Example:

```
/public/images → /etc/
```

Then:

```
/public/images/passwd → /etc/passwd ❌
```

**Conclusion:**
Even valid-looking paths can escape root via symlinks.

---

### 3.5 Why Canonicalization is Needed

* Converts path to a **fully resolved absolute path**
* Removes:

  * `.`
  * `..`
* Resolves symlinks

**Conclusion:**
Canonicalization reveals the *true* filesystem location.

---

### 3.6 Why String Checks Fail

* String comparison is character-based
* Ignores directory boundaries

Example:

```
/public vs /public_malicious
```

```
"/public_malicious".starts_with("/public") → true ❌
```

**Conclusion:**
String prefix checks are unsafe.

---

### 3.7 How to Correctly Verify Containment

Convert paths into structured components:

```
root     = [home, user, project, public]
resolved = [home, user, project, public, images]
```

Check:

* First N components of `resolved`
* Must be **exactly equal** to root components

**Invariant:**

> A resolved path is valid if its first N components are equal to the root components

---

## 4. Final Secure Pipeline

1. Take user request path
2. Strip leading `/` → make it relative
3. Join with root directory
4. Apply canonicalization (`weakly_canonical`)
5. Compare path components:

   * Must match root components exactly
6. If valid:

   * Serve file
7. Else:

   * Reject (403 or 404)

---

## 5. Examples

### Valid Request

```
/images/logo.png
→ images/logo.png
→ /home/user/project/public/images/logo.png ✅
```

---

### Traversal Attack

```
/../../etc/passwd
→ ../../etc/passwd
→ /home/user/etc/passwd ❌ (outside root)
```

---

### Symlink Attack

```
/images/passwd
(images → /etc/)
→ /etc/passwd ❌
```

---

### Prefix Trap

```
/home/user/project/public_malicious/file.txt ❌
```

Even though:

```
starts_with("/home/user/project/public") → true
```

---

## 6. Final Takeaway

Security depends on:

* Treating user input as **untrusted**
* Resolving the **actual filesystem path**
* Verifying **structural containment**, not string similarity

> Always compare paths as structured components, never as raw strings.
