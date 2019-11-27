else if(test_type == "SKELETONTEST")
{
    m_imp = std::make_shared<vts::VTSTestSKELETONTEST>();
    m_imp->load_test_config(config, frontend_cfg);
}
