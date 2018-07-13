
open Typedtree;
open SharedTypes;
open Infix;


module F = (Collector: {let extra: extra;}) => {
  let extra = Collector.extra;

  let addLocation = (loc, ident) => extra.locations = [(loc, ident), ...extra.locations];
  let addReference = (stamp, loc) => Hashtbl.replace(extra.internalReferences, stamp, [loc, ...Hashtbl.mem(extra.internalReferences, stamp) ? Hashtbl.find(extra.internalReferences, stamp) : []]);
  let addExternalReference = (stamp, moduleName, path, loc) => Hashtbl.replace(extra.externalReferences, moduleName, [loc, ...Hashtbl.mem(extra.externalReferences, moduleName) ? Hashtbl.find(extra.externalReferences, moduleName) : []]);

  open Typedtree;
  include TypedtreeIter.DefaultIteratorArgument;
  let enter_structure_item = item => switch (item.str_desc) {
  | Tstr_attribute(({Asttypes.txt: "ocaml.explanation", loc}, PStr([{pstr_desc: Pstr_eval({pexp_desc: Pexp_constant(Const_string(doc, _))}, _)}]))) => {
    addLocation(loc, Loc.Explanation(doc))
  }
  /* | Tstr_type(decls)  */
  | _ => ()
  }
};


let forCmt = (~stamps, {cmt_modname, cmt_annots}: Cmt_format.cmt_infos) => switch cmt_annots {
| Implementation(structure) => {
  let extra = initExtra();
  let addLocation = (loc, ident) => extra.locations = [(loc, ident), ...extra.locations];
  stamps.values |> Hashtbl.iter((stamp, d) => addLocation(d.name.loc, Loc.Typed(d.contents.Value.typ, Loc.ValueDefinition(stamp))));
  stamps.types |> Hashtbl.iter((stamp, d) => {
    addLocation(d.name.loc, Loc.Typed({Types.id: 0, level: 0, desc: Tnil}, Loc.TypeDefinition(stamp)));
    switch (d.contents.Type.kind) {
      | Record(labels) => labels |> List.iter(({Type.Attribute.stamp, name, typ, typLoc}) => addLocation(name.loc, Loc.Typed(typ, Loc.AttributeDefinition(d.stamp, name.txt))));
      | Variant(constructos) => constructos |> List.iter(({Type.Constructor.stamp, name}) => addLocation(name.loc, Loc.Typed({Types.id: 0, level: 0, desc: Tconstr(Path.Pident({Ident.stamp, name: d.name.txt, flags: 0}), [], ref(Types.Mnil))}, Loc.AttributeDefinition(d.stamp, name.txt))));
      | _ => ()
    };
  });
  let module Iter = TypedtreeIter.MakeIterator(F({let extra = extra}));
  List.iter(Iter.iter_structure_item, structure.str_items);
  Some(extra)
}
/* | Interface(signature) => Some(forSignature(processDoc, signature.sig_items)) */
| _ => None
};
