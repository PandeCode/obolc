Running

```bash
nix run github:pandecode/obolc --accept-flake-config
```

## Using this repo as a Nix Flake input

To add this repository as an input to your own Nix flake, add the following to your `flake.nix`:

```nix
{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        nixpkgs-stable.url = "github:nixos/nixpkgs/nixos-25.05";

        obolc.url = "github:PandeCode/obolc";
    };

    nixConfig = {
        accept-flake-config = true;
        extra-substituters = [
          "https://nix-community.cachix.org"
          "https://charon.cachix.org"
        ];
        extra-trusted-public-keys = [
          "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs="
          "charon.cachix.org-1:epdetEs1ll8oi8DT8OG2jEA4whj3FDbqgPFvapEPbY8="
        ];
    };
}
```
