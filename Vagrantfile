# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.define "ubuntu2004_dev" do |ubuntu2004_dev|
    ubuntu2004_dev.vm.box = "generic/ubuntu2004"

    ubuntu2004_dev.vm.synced_folder ".", "/project/streaming-tools",
      owner: "vagrant",
      automount: true,
      id: "streaming-tools"

      ubuntu2004_dev.vm.provider "virtualbox" do |vb|
        vb.gui = true
        vb.memory = "4096"
        vb.cpus = "4"
    end

    # Update repos
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      apt-get update
    SHELL

    # Install all project dependencies
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      apt-get install -y git cmake 
      apt-get install -y libvpx-dev ffmpeg
    SHELL

    # Install desktop environment
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      # some ubuntu-desktop dependency prompts for confirmation
      # and breaks whole VM deployment
      export DEBIAN_FRONTEND=noninteractive
      apt-get install -y ubuntu-desktop
    SHELL

    # Fix terminal
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      localectl set-locale LANG="en_US.UTF-8"
    SHELL

    # Install guest additions
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      sudo apt-get install -y virtualbox-guest-dkms virtualbox-guest-x11 virtualbox-guest-utils
    SHELL

    # Let default user access shared folder
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      if grep -q vboxsf /etc/group
      then
        echo "Group 'vboxsf' exists, adding user to it"
        adduser vagrant vboxsf
      else
        echo "Group 'vboxsf' does not exist"
      fi
    SHELL
    
    # Install Google Chrome
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
      sudo apt install ./google-chrome-stable_current_amd64.deb
    SHELL

    # Install VSCode
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      sudo snap install --classic code
    SHELL

    # Do some useful commands
    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      sudo sysctl net.ipv4.ip_default_ttl=65
      gsettings set org.gnome.shell.extensions.dash-to-dock click-action 'minimize-or-overview'
    SHELL

    ubuntu2004_dev.vm.provision "shell", inline: <<-SHELL
      reboot
    SHELL
  end
end
