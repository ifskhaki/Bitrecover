import json
import os

input_path = r'f:\khaki\download\bitrecover_colab.ipynb'
output_path = r'f:\khaki\crypto\Bitrecover\bitrecover_drive_colab.ipynb'

with open(input_path, 'r', encoding='utf-8') as f:
    nb = json.load(f)

for cell in nb['cells']:
    if cell.get('metadata', {}).get('id') == 'clone_header':
        cell['source'] = ['## 📥 Step 2: Download ZIP from Google Drive & Extract']
    elif cell.get('metadata', {}).get('id') == 'download_config':
        source = [
            "import os\n",
            "\n",
            "# Clean up any previous installation\n",
            "if os.path.exists('/content/bitrecover'):\n",
            "    !rm -rf /content/bitrecover\n",
            "\n",
            "# Navigate to colab content dir\n",
            "%cd /content\n",
            "\n",
            "# Install gdown for Google Drive downloads\n",
            "!pip install -q gdown\n",
            "import gdown\n",
            "\n",
            "# ═══════════════════════════════════════════════════════════════\n",
            "# IMPORTANT: To avoid the Google Drive 'file limit error', do NOT use a folder link.\n",
            "# 1. Compress your 'bitrecover' folder into 'bitrecover.zip' on your computer.\n",
            "# 2. Upload 'bitrecover.zip' to Google Drive.\n",
            "# 3. Get the shareable link for 'bitrecover.zip' and paste the FILE ID below.\n",
            "# ═══════════════════════════════════════════════════════════════\n",
            "zip_file_id = \"ENTER_YOUR_ZIP_FILE_ID_HERE\"  # CHANGE THIS TO YOUR ZIP FILE ID!\n",
            "\n",
            "try:\n",
            "    print(\"📥 Downloading bitrecover.zip...\")\n",
            "    gdown.download(f\"https://drive.google.com/uc?id={zip_file_id}\", \"bitrecover.zip\", quiet=False)\n",
            "    \n",
            "    print(\"📦 Extracting bitrecover.zip...\")\n",
            "    !unzip -q bitrecover.zip -d /content/\n",
            "    !rm bitrecover.zip\n",
            "    print(\"✅ Project downloaded and extracted successfully!\")\n",
            "except Exception as e:\n",
            "    print(f\"❌ ERROR: Failed to download or extract ZIP: {e}\")\n",
            "\n",
            "%cd /content/bitrecover\n",
            "\n",
            "# ═══════════════════════════════════════════════════════════════\n",
            "# IMPORTANT: Update these Google Drive file IDs with your config & address!\n",
            "# ═══════════════════════════════════════════════════════════════\n",
            "config_file_id = \"1t3Px1xZtNK2ntntEpK0lZyyBCGRjLz69\"  # CHANGE THIS!\n",
            "address_file_id = \"1NzoV0xC75_t_o9zR33YBH54uo4iWDTeh\"  # CHANGE THIS!\n",
            "# Download and replace config.json\n",
            "try:\n",
            "    os.makedirs('config', exist_ok=True)\n",
            "    gdown.download(f\"https://drive.google.com/uc?id={config_file_id}\", \"config/config.json\", quiet=False)\n",
            "    print(\"✅ Downloaded your config.json from Google Drive\")\n",
            "except Exception as e:\n",
            "    print(f\"⚠️ Error downloading config: {e}\")\n",
            "\n",
            "# Download and replace address.txt\n",
            "try:\n",
            "    gdown.download(f\"https://drive.google.com/uc?id={address_file_id}\", \"address.txt\", quiet=False)\n",
            "    print(\"✅ Downloaded your address.txt from Google Drive\")\n",
            "except Exception as e:\n",
            "    print(f\"⚠️ Error downloading address.txt: {e}\")\n",
            "\n",
            "print(\"\\n✅ All configuration files ready!\")\n"
        ]
        cell['source'] = source

with open(output_path, 'w', encoding='utf-8') as f:
    json.dump(nb, f, indent=2)

print(f"Created updated notebook at: {output_path}")
