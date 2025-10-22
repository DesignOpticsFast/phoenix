#!/bin/bash

# Connect to Mac and Discover Environment
# Purpose: Connect to Mac via Tailscale and discover Qt6 installation

echo "🔗 CONNECTING TO MAC VIA TAILSCALE"
echo "=================================="
echo ""

# Check if SSH key exists
if [ ! -f ~/.ssh/github_phoenix ]; then
    echo "❌ SSH key not found: ~/.ssh/github_phoenix"
    echo "Please ensure the SSH key is available"
    exit 1
fi

echo "SSH Key found: ~/.ssh/github_phoenix"
echo ""

# Test connection
echo "Testing connection to Mac..."
ssh -i ~/.ssh/github_phoenix -o ConnectTimeout=10 -o StrictHostKeyChecking=no mark@100.97.54.75 "echo 'Connection successful'" 2>/dev/null

if [ $? -eq 0 ]; then
    echo "✅ Connection to Mac successful"
    echo ""
    
    echo "🚀 Starting Mac environment discovery..."
    echo "======================================"
    echo ""
    
    # Copy discovery script to Mac and run it
    echo "Copying discovery script to Mac..."
    scp -i ~/.ssh/github_phoenix /home/ec2-user/workspace/phoenix/scripts/discover_mac_environment.sh mark@100.97.54.75:~/discover_mac_environment.sh
    
    echo "Running discovery script on Mac..."
    ssh -i ~/.ssh/github_phoenix mark@100.97.54.75 "chmod +x ~/discover_mac_environment.sh && ~/discover_mac_environment.sh"
    
    echo ""
    echo "📋 Next Steps:"
    echo "1. Review the discovery output above"
    echo "2. Identify the correct Qt6 installation path"
    echo "3. Update MAC_ENVIRONMENT_DOCUMENTATION.md with findings"
    echo "4. Configure CMake with correct CMAKE_PREFIX_PATH"
    echo "5. Test Phoenix build on Mac"
    
else
    echo "❌ Connection to Mac failed"
    echo ""
    echo "Troubleshooting:"
    echo "1. Check Tailscale connection: tailscale status"
    echo "2. Verify SSH key permissions: chmod 600 ~/.ssh/github_phoenix"
    echo "3. Test manual connection: ssh -i ~/.ssh/github_phoenix mark@100.97.54.75"
    echo "4. Check if Mac is online and accessible"
fi

