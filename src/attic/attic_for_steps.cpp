/** If there's a structure of gotos and labels that looks like a switch statement
    then insert a switch statement, turn labels into enums and the type of the
    control variable from void * to the new enum */
class InsertSwitch : public SearchReplace
{
public:
    InsertSwitch();
};


InsertSwitch::InsertSwitch()
{
    MakePatternPtr<Instance> fn;
    MakePatternPtr<InstanceIdentifier> fn_id;
    MakePatternPtr<Callable> sub;
    MakePatternPtr< Overlay<Compound> > func_over, over, l_over;
    MakePatternPtr< Compound > ls_func_comp, lr_func_comp, s_func_comp, r_func_comp, s_comp, r_comp, r_switch_comp, l_comp, ls_switch_comp, lr_switch_comp;
    MakePatternPtr< Star<Declaration> > func_decls, decls, l_enum_vals;
    MakePatternPtr< Star<Statement> > func_pre, func_post, pre, body, l_func_pre, l_func_post, l_pre, l_post;
    MakePatternPtr< Stuff<Statement> > stuff, l_stuff; // TODO these are parallel stuffs, which is bad. Use two first-level embeddeds 
                                                    // and modify S&R to allow couplings between them. This means running embeddeds 
                                                    // in a post-pass and doing existing passes across all same-level embeddeds
    MakePatternPtr<Goto> s_first_goto; 
    MakePatternPtr<Label> ls_label; 
    MakePatternPtr<Switch> r_switch, l_switch;     
    MakePatternPtr<Enum> r_enum, ls_enum, lr_enum;         
    MakePatternPtr< Negation<Statement> > s_prenot, s_postnot, xs_rr;
    MakePatternPtr<BuildTypeIdentifierAgent> r_enum_id("%sStates");
    MakePatternPtr<Static> lr_state_decl;    
    MakePatternPtr<BuildInstanceIdentifierAgent> lr_state_id("STATE_%s");
    MakePatternPtr<Case> lr_case;
    MakePatternPtr<Signed> lr_int;
    MakePatternPtr<BuildContainerSizeAgent> lr_count;
    MakePatternPtr<IsLabelReachedAgent> ls_label_id;
    MakePatternPtr<InstanceIdentifier> var_id;
    MakePatternPtr<Instance> var_decl, l_var_decl;
    MakePatternPtr< Overlay<Type> > var_over;  
    MakePatternPtr<Label> xs_pre_label;
    MakePatternPtr<IsLabelReachedAgent> xs_pre_reach;
    MakePatternPtr< Conjunction<Node> > ll_all;
    MakePatternPtr< Negation<Node> > lls_not1, lls_not2;    
    MakePatternPtr< AnyNode<Node> > ll_any;
    MakePatternPtr< Overlay<Node> > ll_over;
    MakePatternPtr<Goto> lls_goto;    
    MakePatternPtr<Label> lls_label;    
            
    fn->type = sub;
    fn->initialiser = func_over;
    fn->identifier = fn_id;
    func_over->through = s_func_comp;
    s_func_comp->members = (func_decls, var_decl);
    var_decl->type = var_over;
    var_decl->identifier = var_id;
    var_over->through = MakePatternPtr<Labeley>();
    s_func_comp->statements = (func_pre, stuff, func_post);
    stuff->terminus = over;
    stuff->recurse_restriction = xs_rr; // TODO Add support for elsewhere restriction in stuff node, restrict for no reaches
    xs_rr->pattern = MakePatternPtr<Switch>(); // stop it doing a second switch inside one we just created
    over->through = s_comp;
    s_comp->members = (decls);
    s_comp->statements = (pre, s_first_goto, body);
    pre->restriction = s_prenot;
    s_prenot->pattern = xs_pre_label;
    xs_pre_label->identifier = xs_pre_reach;
    xs_pre_reach->pattern = var_id;
    s_first_goto->destination = var_id;

    r_func_comp->members = (func_decls, r_enum, var_decl);
    var_over->overlay = r_enum_id;
    r_enum->identifier = r_enum_id;
    r_enum_id->sources = (fn_id);        
    r_func_comp->statements = (func_pre, stuff, func_post);
    over->overlay = r_comp;
    r_comp->members = (decls);
    r_comp->statements = (pre, r_switch);    
    r_switch->body = r_switch_comp;
    r_switch_comp->statements = (body);
    r_switch->condition = var_id;

    MakePatternPtr< SlaveSearchReplace<Compound> > lr_sub_embedded( lr_func_comp, ll_all );    
    MakePatternPtr< SlaveCompareReplace<Compound> > r_embedded( r_func_comp, ls_func_comp, lr_sub_embedded );
    func_over->overlay = r_embedded;
    ls_func_comp->members = (func_decls, ls_enum, l_var_decl);
    ls_enum->members = (l_enum_vals);
    ls_enum->identifier = r_enum_id; // need to match id, not enum itself, because enum's members will change during embedded
    ls_func_comp->statements = (l_func_pre, l_stuff, l_func_post);
    l_stuff->terminus = l_comp;
    l_comp->members = (decls);
    l_comp->statements = (pre, l_switch);
    l_switch->body = l_over;
    l_switch->condition = s_first_goto->destination;
    l_over->through = ls_switch_comp;
    ls_switch_comp->statements = (l_pre, ls_label, l_post);
    ls_label->identifier = ls_label_id;
    ls_label_id->pattern = var_id;
    
    lr_func_comp->members = (func_decls, lr_enum, l_var_decl);
    lr_enum->members = (l_enum_vals, lr_state_decl);
    lr_enum->identifier = r_enum_id;
    lr_state_decl->constancy = MakePatternPtr<Const>();
    lr_state_decl->identifier = lr_state_id;
    lr_state_decl->type = lr_int;
    lr_state_decl->initialiser = lr_count;
    lr_count->container = l_enum_vals;
    lr_int->width = MakePatternPtr<SpecificInteger>(32); // TODO should be a common place for getting default types
    lr_state_id->sources = (ls_label->identifier);
    lr_func_comp->statements = (l_func_pre, l_stuff, l_func_post);
    l_over->overlay = lr_switch_comp;
    lr_switch_comp->statements = (l_pre, lr_case, /*ls_label,*/ l_post); 
    // TODO retain the label for direct gotos, BUT we are spinning at 1st embedded because we think the
    // label is still a state, because we think it reaches state var, because the LABEL->STATE_LABEL change 
    // is not being seen by IsLabelReachedAgent, because this is done by 2nd embedded and stays in temps until
    // the master's SingleCompareReplace() completes. Uh-oh!
    // Only fix after fixing S&R to ensure embedded output goes into context
    lr_case->value = lr_state_id;

    ll_all->patterns = (ll_any, lls_not1, lls_not2);
    ll_any->terminus = ll_over;
    ll_over->through = ls_label_id;
    ll_over->overlay = lr_state_id;
    lls_not1->pattern = lls_goto;
    lls_goto->destination = ls_label_id; // leave gotos alone in the body
    lls_not2->pattern = lls_label;
    lls_label->identifier = ls_label_id; // leave labels alone in the body

    Configure( fn );    
}


