# ✅ How to Upload .github Folder to GitHub

## What is `.github`?

The `.github` folder contains GitHub-specific files like:
- **Workflows**: Automated CI/CD pipelines (GitHub Actions)
- **Templates**: Issue and PR templates
- **Config**: Repository settings

## ✅ I've Already Copied It For You!

I just copied `.github/workflows/build.yml` into your bitrecover folder.

---

## 📤 How to Upload to GitHub

### Method 1: Command Line (Recommended)

```bash
cd "f:\khaki\crypto\working on\bitrecover"

# Add the .github folder
git add .github/

# Commit
git commit -m "Add GitHub Actions CI/CD workflow"

# Push to GitHub
git push origin main
```

### Method 2: GitHub Desktop

1. Open GitHub Desktop
2. Select your Bitrecover repository
3. You'll see `.github/workflows/build.yml` in changes
4. Check the box next to it
5. Write commit message: "Add GitHub Actions workflow"
6. Click "Commit to main"
7. Click "Push origin"

### Method 3: Manual Upload (Not Recommended)

1. Go to https://github.com/ifskhaki/Bitrecover
2. Click "Add file" → "Upload files"
3. Drag `.github` folder
4. Commit changes

---

## 🔍 Verify It Worked

After pushing, check:
1. Go to https://github.com/ifskhaki/Bitrecover
2. You should see `.github` folder
3. Go to "Actions" tab
4. You should see "Build and Test" workflow

---

## ⚡ What This Workflow Does

Your `build.yml` will:
- ✅ Auto-build on every push
- ✅ Install CUDA toolkit
- ✅ Run CMake
- ✅ Compile bitrecover
- ✅ Test `--list-devices`

**This gives your repo a professional ✅ badge!**

---

## 🎯 Quick Fix

Run this in PowerShell:

```powershell
cd "f:\khaki\crypto\working on\bitrecover"
git add .github/
git commit -m "Add CI/CD workflow"
git push
```

Done! ✅
