{
  let foo = "closure"
  fn f {
    {
      print foo // [Expect] closure
      let foo = "shadow"
      print foo // [Expect] shadow
    }
    print foo // [Expect] closure
  }
  f()
}
