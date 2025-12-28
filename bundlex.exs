defmodule Shmex.BundlexProject do
  use Bundlex.Project

  def project do
    os = Bundlex.get_target().os

    [
      natives: natives(os),
      libs: libs(os)
    ]
  end

  defp natives("windows") do
    [
      shmex: [
        interface: :nif,
        deps: [shmex: :shmex_nif, bunch_native: :bunch],
        sources: ["shmex.c"]
      ]
    ]
  end

  defp natives(_) do
    [
      shmex: [
        interface: :nif,
        deps: [shmex: :shmex, bunch_native: :bunch],
        sources: ["shmex.c"]
      ]
    ]
  end

  defp libs("windows") do
    [
      lib: [
        src_base: "shmex/shmex",
        sources: ["lib_win.c"],
        libs: ["ole32.lib"]
      ],
      shmex_nif: [
        interface: :nif,
        deps: [shmex: :lib, bunch_native: :bunch],
        src_base: "shmex/nif/shmex",
        sources: ["shmex.c"]
      ],
      shmex: [
        interface: :cnode,
        deps: [shmex: :lib],
        src_base: "shmex/cnode/shmex",
        sources: ["shmex.c"]
      ]
    ]
  end

  defp libs(_) do
    [
      lib: [
        src_base: "shmex/shmex",
        sources: ["lib.c"],
        libs: if(Bundlex.get_target().os == "linux", do: ["rt"], else: [])
      ],
      shmex: [
        interface: :nif,
        deps: [shmex: :lib, bunch_native: :bunch],
        src_base: "shmex/nif/shmex",
        sources: ["shmex.c"]
      ],
      shmex: [
        interface: :cnode,
        deps: [shmex: :lib],
        src_base: "shmex/cnode/shmex",
        sources: ["shmex.c"]
      ]
    ]
  end
end
