
#include "sliding_shape.h"
#include "version.h"
#include <stdio.h>

#define WARNING "The input tab file must be sorted (generated by sam2tab)"

void print_usage_root()
{
    char buff[3000];
    const char *help_info = 
            "calc_sliding_shape - calculate SHAPE score with a sliding window\n"
            "=============================================================\n"
            "\e[1mUSAGE:\e[0m\n"
            "\tsliding_shape [mode] [options]\n"
            "\e[1mHELP:\e[0m\n"
            "\t[mode]\n"
            "\t\tTrtCont: calculate SHAPE score with Treatment (NAI-N3) and Control (DMSO) samples \n"
            "\t\tTrt: calculate SHAPE score with Treatment (NAI-N3) samples\n\n"

            "\e[1mWARNING:\e[0m\n\t%s\n"
            "\e[1mVERSION:\e[0m\n\t%s\n"
            "\e[1mLIB VERSION:\e[0m\n\t%s\n"
            "\e[1mDATE:\e[0m\n\t%s\n"
            "\e[1mAUTHOR:\e[0m\n\t%s\n";
    
    ostringstream warning;
    warning << YELLOW << WARNING << DEF;

    sprintf(buff, help_info, warning.str().c_str(), BINVERSION, LIBVERSION, DATE, "Li Pan");
    cout << buff << endl;
}

void print_usage_smartSHAPE()
{
    char buff[3000];
    const char *help_info = 
            "calc_sliding_shape Trt - calculate SHAPE score with a sliding window with Treatment (NAI-N3) samples\n"
            "====================================================================================================\n"
            "\e[1mUSAGE:\e[0m\n"
            "\tsliding_shape Trt -N inputTab1,inputTab2... -size chrSizeFile [-ijf junctionFile] -out out.gTab\n"
            "\e[1mHELP:\e[0m\n"
            "\t-N: input Treatment (NAI-N3) tab files generated by sam2tab (must be sorted)\n"
            "\t-size: input a chromosome size file generated by STAR (chrNameLength.txt) \n"
            "\t-ijf: input a junction file generated by STAR (sjdbList.fromGTF.out.tab)\n"
            "\t-ojf: output a junction file with sopported reads\n"
            "\t-out: output RT, BD and shape scores\n"
            "\t-noparam: don't output a parameter file\n\n"

            "\t-wsize: window size (default: 200)\n"
            "\t-wstep: window step (default: 5)\n"
            "\t-enm: <rt/div/log> enrich method. RT means use raw RT to calculate SHAPE;\n"
            "\t      div means use RT/BD to calculate SHAPE;\n"
            "\t      log means use log2(RT+add_fac) to calculate SHAPE (default: rt)\n"
            "\t-ext: BD extend (default: 0)\n"
            "\t-sink: sink index (default: 0.0)\n"
            "\t-wf: winsor factor (default: 0.05)\n"
            "\t-sf: substract factor (default: 0.25)\n"
            "\t-df: division factor (default: 10)\n"
            "\t-af: add factor (default: 1)\n"
            "\t-omc: minimul NAI BD for output (default: 50)\n"
            "\t-mc: minimul NAI BD for valid shape score (default: 100)\n\n"

            "\t-genome: genome fasta file, base will be output (default: None)\n"
            "\t-bases: base to calculate shape scores, such as A,C,a,c, -genome should be provided\n"
            "\t-non-sliding: Calculate the shape score as a whole, without using the sliding window (default: None)\n"
            "\t              Suitable for short RNAs. Some parameters will be ignored in this case\n"
            "\t-separate: Each base is calculated separately (default: None) -genome/-bases should be provided\n\n"

            "\e[1mWARNING:\e[0m\n\t%s\n"
            "\e[1mVERSION:\e[0m\n\t%s\n"
            "\e[1mLIB VERSION:\e[0m\n\t%s\n"
            "\e[1mDATE:\e[0m\n\t%s\n"
            "\e[1mAUTHOR:\e[0m\n\t%s\n";
    
    ostringstream warning;
    warning << YELLOW << WARNING << DEF;

    sprintf(buff, help_info, warning.str().c_str(), BINVERSION, LIBVERSION, DATE, "Li Pan");
    cout << buff << endl;
}

void print_usage_icSHAPE()
{
    char buff[3000];
    const char *help_info = 
            "calc_sliding_shape TrtCont - calculate SHAPE score with a sliding window with Treatment (NAI-N3) and Control (DMSO) samples\n"
            "===========================================================================================================================\n"
            "\e[1mUSAGE:\e[0m\n"
            "\tsliding_shape TrtCont -D inputTab1,inputTab2... -N inputTab1,inputTab2... -size chrSizeFile [-ijf junctionFile] -out out.gTab\n"
            "\e[1mHELP:\e[0m\n"
            "\t-D: input Control (DMSO) tab files generated by sam2tab (must be sorted)\n"
            "\t-N: input Treatment (NAI-N3) tab files generated by sam2tab (must be sorted)\n"
            "\t-size: input a chromosome size file generated by STAR (chrNameLength.txt) \n"
            "\t-ijf: input a junction file generated by STAR (sjdbList.fromGTF.out.tab)\n"
            "\t-ojf: output a junction file with sopported reads\n"
            "\t-out: output RT/BD/shape scores\n"
            "\t-noparam: don't output a parameter file\n\n"

            "\t-wsize: window size (default: 200)\n"
            "\t-wstep: window step (default: 5)\n"
            "\t-enm: <sub/div/complex/log> enrich method.(default: complex)\n" 
                "\t\t sub: nai_rt-sub_fac*dmso_rt \n"
                "\t\t div: div_fac*nai_bd/dmso_bd \n"
                "\t\t complex: div_fac*(nai_rt-sub_fac*dmso_rt)/dmso_bd \n"
                "\t\t log: log2( (nai_rt+add_fac)/(dmso_rt+add_fac) ) \n"
            "\t-nom: <sm/upp/qua/dec/vigi> normalization method.(default: dec)\n" 
                "\t\t sm: all sorted non-zero values \n"
                "\t\t upp: upper half part of all sorted non-zero values \n"
                "\t\t qua: (n-1)/4 ~ n/4 part of all sorted values \n"
                "\t\t dec: (n-1)/10 ~ n/10 part of all sorted values \n"
                "\t\t vigi: (n-1)/20 ~ n/20 part of all sorted values \n"
            "\t-nomi: normalization index. (default: 2)\n"
            "\t-nocm: <mean/median/peak> calculate a scalling factor from sampled data.(default: median)\n" 
                "\t\t mean: mean value \n"
                "\t\t median: median value \n"
                "\t\t peak: max value \n"
            "\t-wf: winsor factor (default: 0.05)\n"
            "\t-sf: substract factor (default: 0.25)\n"
            "\t-df: division factor (default: 10)\n"
            "\t-af: add factor (default: 1)\n"
            "\t-omc: minimul DMSO BD for output (default: 50)\n"
            "\t-mc: minimul DMSO BD for valid shape score (default: 100)\n\n"

            "\t-genome: genome fasta file (default: None)\n"
            "\t-bases: base to calculate shape scores, such as A,C,a,c, -genome should be provided\n"
            "\t-non-sliding: Calculate the shape score as a whole, without using the sliding window (default: None)\n"
            "\t              Suitable for short RNAs. Some parameters will be ignored in this case\n"
            "\t-separate: Each base is calculated separately (default: None), -genome/-bases should be provided\n\n"

            "\e[1mWARNING:\e[0m\n\t%s\n"
            "\e[1mVERSION:\e[0m\n\t%s\n"
            "\e[1mLIB VERSION:\e[0m\n\t%s\n"
            "\e[1mDATE:\e[0m\n\t%s\n"
            "\e[1mAUTHOR:\e[0m\n\t%s\n";
    
    ostringstream warning;
    warning << YELLOW << WARNING << DEF;

    sprintf(buff, help_info, warning.str().c_str(), BINVERSION, LIBVERSION, DATE, "Li Pan");
    cout << buff << endl;
}

void print_usage(int argc, char *argv[])
{
    if(argc<2)
        print_usage_root();
    else if(not strcmp(argv[1], "TrtCont"))
        print_usage_icSHAPE();
    else if(not strcmp(argv[1], "Trt"))
        print_usage_smartSHAPE();
    else
        print_usage_root();
}

enum Prog_MODE { TrtCont=0, Trt=1, Unknown=3 };

void check_norm_sample_method(NormSampleMethod &v, const string &option)
{
    if(option == "sm")
        v = SMART;
    else if(option == "upp")
        v = UPPER;
    else if(option == "qua")
        v = QUARTILE;
    else if(option == "dec")
        v = DECILE;
    else if(option == "vigi")
        v = VIGINTILE;
    else{
        cerr << RED << "FATAL Error: Unknown -nom " << option << DEF << endl;
        exit(-1);
    }
}

void check_norm_calc_method(NormCalcMethod &v, const string &option)
{
    if(option == "sm")
        v = MEDIAN;
    else if(option == "upp")
        v = MEAN;
    else if(option == "qua")
        v = PEAK;
    else{
        cerr << RED << "FATAL Error: Unknown -nocm " << option << DEF << endl;
        exit(-1);
    }
}

void check_enrich_method(EnrichMethod &v, const string &option)
{
    if(option == "sub")
        v = SUBSTRACTION;
    else if(option == "div")
        v = DIVIDING;
    else if(option == "complex")
        v = COMPLEX;
    else if(option == "rt")
        v = RT;
    else if(option == "log")
        v = LOG;
    else{
        cerr << RED << "FATAL Error: Unknown -enm " << option << DEF << endl;
        exit(-1);
    }
}

struct General_Param
{

    /*  Generaral Part  */
    //string D1, D2, N1, N2;
    StringArray DFiles;
    StringArray NFiles;

    string size_file;
    string junction_file;
    string out_junc_file;
    string out_file;

    string genome_seq_file;
    vector<char> bases;
    bool base_separate = false;

    Prog_MODE mode=Unknown;

    bool noParam = false;

    /* Common Part */

    float winsor_factor = 0.05;
    float sub_factor = 0.25;
    float div_factor = 10;
    float add_factor = 1;
    uINT out_min_cov = 50;
    uINT min_cov = 100;
    uINT wsize = 200;
    uINT wstep = 5;
    bool no_sliding = false;

    /* icSHAPE Part */

    NormSampleMethod norm_sample_method = DECILE;
    NormCalcMethod norm_calc_method = MEDIAN;
    uINT norm_sample_factor = 2;
    EnrichMethod icSHAPE_enrich_method = COMPLEX;
    uINT winsor_scaling = 1;

    /* smartSHAPE Part */

    float sink_index = 0;
    EnrichMethod smartSHAPE_enrich_method = RT;
    uLONG BD_ext = 0;

    operator bool()
    {
        if(mode == Unknown)
            return false;

        if(size_file.empty() or out_file.empty() or NFiles.empty())
            return false;

        if(mode==TrtCont and DFiles.empty())
        {
            cerr << RED << "FATAL Error: specify -D in [TrtCont] mode" << DEF << endl;
            return false;
        }

        if(mode==Trt and (not DFiles.empty()))
            cerr << RED << "Warning: -D is ignored in [Trt] mode" << DEF << endl;

        if(not bases.empty() and genome_seq_file.empty())
        {
            cerr << RED << "FATAL Error: -genome should be provided when -bases specified" << DEF << endl;
            return false;
        }

        if(base_separate and (bases.empty() or genome_seq_file.empty()))
        {
            cerr << RED << "FATAL Error: -bases and -genome should be provided when -separate specified" << DEF << endl;
            return false;
        }

        return true;
    }
};

icSHAPE_Param build_icSHAPE_param(const General_Param &param)
{
    icSHAPE_Param shape_param;

    shape_param.norm_sample_method = param.norm_sample_method;
    shape_param.norm_calc_method = param.norm_calc_method;
    shape_param.norm_sample_factor = param.norm_sample_factor;
    shape_param.enrich_method = param.icSHAPE_enrich_method;
    shape_param.winsor_factor = param.winsor_factor;
    shape_param.winsor_scaling = param.winsor_scaling;
    shape_param.sub_factor = param.sub_factor;
    shape_param.div_factor = param.div_factor;
    shape_param.add_factor = param.add_factor;
    shape_param.out_min_cov = param.out_min_cov;
    shape_param.min_cov = param.min_cov;
    shape_param.wsize = param.wsize;
    shape_param.wstep = param.wstep;
    shape_param.no_sliding = param.no_sliding;

    return shape_param;
}

smartSHAPE_Param build_smartSHAPE_param(const General_Param &param)
{
    smartSHAPE_Param shape_param;

    shape_param.enrich_method = param.smartSHAPE_enrich_method;
    shape_param.winsor_factor = param.winsor_factor;
    shape_param.sub_factor = param.sub_factor;
    shape_param.div_factor = param.div_factor;
    shape_param.add_factor = param.add_factor;
    shape_param.out_min_cov = param.out_min_cov;
    shape_param.min_cov = param.min_cov;
    shape_param.wsize = param.wsize;
    shape_param.wstep = param.wstep;
    shape_param.BD_ext = param.BD_ext;
    shape_param.sink_index = param.sink_index;
    shape_param.no_sliding = param.no_sliding;

    return shape_param;
}

void has_next(int argc, char *argv[], int current)
{
    if(current + 1 >= argc)
    {
        cerr << RED << "FATAL ERROR: Parameter Error" << DEF << endl;
        print_usage(argc, argv);
        exit(-1);
    }
}

General_Param read_param(int argc, char *argv[])
{
    General_Param param;

    if(argc <= 1)
    {
        print_usage(argc, argv);
        exit(-1);
    }

    if(not strcmp(argv[1], "TrtCont"))
        param.mode = TrtCont;
    else if(not strcmp(argv[1], "Trt"))
        param.mode = Trt;
    else{
        cerr << RED << "FATAL Error: unrecognized mode: " << argv[1] << endl;
        exit(-1);
    }

    for(int i=2; i<argc; i++)
    {
        if( argv[i][0] == '-' )
        {
            if(not strcmp(argv[i]+1, "D"))
            {
                has_next(argc, argv, i);
                split(argv[i+1], ',', param.DFiles);
                i++;
            }else if(not strcmp(argv[i]+1, "N"))
            {
                has_next(argc, argv, i);
                split(argv[i+1], ',', param.NFiles);
                i++;
            }else if(not strcmp(argv[i]+1, "ijf"))
            {
                has_next(argc, argv, i);
                param.junction_file = argv[i+1];
                i++;
            }else if(not strcmp(argv[i]+1, "ojf"))
            {
                has_next(argc, argv, i);
                param.out_junc_file = argv[i+1];
                i++;
            }else if(not strcmp(argv[i]+1, "size"))
            {
                has_next(argc, argv, i);
                param.size_file = argv[i+1];
                i++;
            }else if(not strcmp(argv[i]+1, "out"))
            {
                has_next(argc, argv, i);
                param.out_file = argv[i+1];
                i++;
            }else if(not strcmp(argv[i]+1, "noparam"))
            {
                //has_next(argc, i);
                param.noParam = true;
                //i++;
            }

            else if(not strcmp(argv[i]+1, "wsize"))
            {
                has_next(argc, argv, i);
                param.wsize = stoul(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "wstep"))
            {
                has_next(argc, argv, i);
                param.wstep = stoul(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "enm"))
            {
                has_next(argc, argv, i);
                check_enrich_method(param.icSHAPE_enrich_method, argv[i+1]);
                check_enrich_method(param.smartSHAPE_enrich_method, argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "ext"))
            {
                has_next(argc, argv, i);
                param.BD_ext = stoul(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "sink"))
            {
                has_next(argc, argv, i);
                param.sink_index = stof(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "wf"))
            {
                has_next(argc, argv, i);
                param.winsor_factor = stof(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "sf"))
            {
                has_next(argc, argv, i);
                param.sub_factor = stof(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "df"))
            {
                has_next(argc, argv, i);
                param.div_factor = stof(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "af"))
            {
                has_next(argc, argv, i);
                param.add_factor = stof(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "omc"))
            {
                has_next(argc, argv, i);
                param.out_min_cov = stoul(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "mc"))
            {
                has_next(argc,argv, i);
                param.min_cov = stoul(argv[i+1]);
                i++;
            }

            else if(not strcmp(argv[i]+1, "nom"))
            {
                has_next(argc, argv, i);
                check_norm_sample_method(param.norm_sample_method, argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "nomi"))
            {
                has_next(argc, argv, i);
                param.norm_sample_factor = stoul(argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "nocm"))
            {
                has_next(argc, argv, i);
                check_norm_calc_method(param.norm_calc_method, argv[i+1]);
                i++;
            }

            else if(not strcmp(argv[i]+1, "genome"))
            {
                has_next(argc, argv, i);
                param.genome_seq_file = argv[i+1];
                //check_norm_calc_method(param.norm_calc_method, argv[i+1]);
                i++;
            }else if(not strcmp(argv[i]+1, "bases"))
            {
                has_next(argc, argv, i);
                StringArray array;
                split(argv[i+1], ',', array);
                for(string base: array)
                    param.bases.push_back(base[0]);
                i++;
            }else if(not strcmp(argv[i]+1, "non-sliding"))
            {
                //has_next(argc, argv, i);
                param.no_sliding = true;
                //i++;
            }else if(not strcmp(argv[i]+1, "separate"))
            {
                //has_next(argc, argv, i);
                param.base_separate = true;
                //i++;
            }

            else{
                cerr << RED << "FATAL ERROR: unknown option: " << argv[i] << DEF << endl;
                print_usage(argc, argv);
                exit(-1);
            }
        }else{
            cerr << RED << "FATAL ERROR: unknown option: " << argv[i] << DEF << endl;
            print_usage(argc, argv);
            exit(-1);
        }
    }
    return param;
}



void record_param(const General_Param &param, const int argc, char *argv[])
{
    ofstream OUT(param.out_file+".param.log", ofstream::out);

    OUT << "Prog Compile version: " << __DATE__ << "\n";
    OUT << "Run Time: " << currentDateTime() << "\n";

    for(int i=0; i<argc; i++)
        OUT << argv[i] << " ";
    OUT << "\n";

    if(param.mode == TrtCont)
    {
        OUT << "Calculate with [TrtCont] mode" << "\n";

        OUT << "\t DMSO files: " << param.DFiles << "\n";
        OUT << "\t NAI files: " << param.NFiles << "\n";

        OUT << "\t chromosome size file: " << param.size_file << "\n";
        OUT << "\t junction file: " << param.junction_file << "\n";
        OUT << "\t output junction file: " << param.out_junc_file << "\n";
        OUT << "\t output file: " << param.out_file << "\n\n";

        OUT << "Use sliding window: " << (param.no_sliding ? "No" : "Yes") << "\n\n";

        OUT << "\t winsor_factor: " << param.winsor_factor << "\n";
        OUT << "\t sub_factor: " << param.sub_factor << "\n";
        OUT << "\t div_factor: " << param.div_factor << "\n";
        OUT << "\t add_factor: " << param.add_factor << "\n";
        OUT << "\t out_min_cov: " << param.out_min_cov << "\n";
        OUT << "\t min_cov: " << param.min_cov << "\n";
        OUT << "\t window size: " << param.wsize << "\n";
        OUT << "\t window step: " << param.wstep << "\n\n";

        if(not param.genome_seq_file.empty())
        {
            OUT << "\t genome file: " << param.genome_seq_file << "\n";
            OUT << "\t bases: " << param.bases << "\n";
            OUT << "\t seperate calculate: " << (param.base_separate ? "Yes" : "No") << "\n\n";
        }

        OUT << "\t norm_sample_method: ";
        if(param.norm_sample_method == SMART)
            OUT << "smart" << "\n";
        else if(param.norm_sample_method == UPPER)
            OUT << "upper" << "\n";
        else if(param.norm_sample_method == QUARTILE)
            OUT << "quantile" << "\n";
        else if(param.norm_sample_method == DECILE)
            OUT << "decile" << "\n";
        else if(param.norm_sample_method == VIGINTILE)
            OUT << "vigintile" << "\n";

        OUT << "\t norm_calc_method: ";
        if(param.norm_calc_method == MEDIAN)
            OUT << "median" << "\n";
        else if(param.norm_calc_method == MEAN)
            OUT << "mean" << "\n";
        else if(param.norm_calc_method == PEAK)
            OUT << "peak" << "\n";

        OUT << "\t norm_sample_factor: " << param.norm_sample_factor << "\n";

        OUT << "\t enrich_method: ";
        if(param.icSHAPE_enrich_method == SUBSTRACTION)
            OUT << "substraction" << "\n";
        else if(param.icSHAPE_enrich_method == DIVIDING)
            OUT << "dividing" << "\n";
        else if(param.icSHAPE_enrich_method == COMPLEX)
            OUT << "complex" << "\n";
        else if(param.icSHAPE_enrich_method == RT)
            OUT << "rt" << "\n";
        else if(param.icSHAPE_enrich_method == LOG)
            OUT << "log" << "\n";

        OUT << "\t winsor_scaling: " << param.winsor_scaling << "\n\n";

    }else if(param.mode == Trt){

        OUT << "Calculate with [Trt] mode" << "\n";

        OUT << "\t NAI files: " << param.NFiles << "\n";

        OUT << "\t chromosome size file: " << param.size_file << "\n";
        OUT << "\t junction file: " << param.junction_file << "\n";
        OUT << "\t output junction file: " << param.out_junc_file << "\n";
        OUT << "\t output file: " << param.out_file << "\n\n";

        OUT << "Use sliding window: " << (param.no_sliding ? "No" : "Yes") << "\n\n";

        OUT << "\t winsor_factor: " << param.winsor_factor << "\n";
        OUT << "\t sub_factor: " << param.sub_factor << "\n";
        OUT << "\t div_factor: " << param.div_factor << "\n";
        OUT << "\t add_factor: " << param.add_factor << "\n";
        OUT << "\t out_min_cov: " << param.out_min_cov << "\n";
        OUT << "\t min_cov: " << param.min_cov << "\n";
        OUT << "\t window size: " << param.wsize << "\n";
        OUT << "\t window step: " << param.wstep << "\n\n";

        if(not param.genome_seq_file.empty())
        {
            OUT << "\t genome file: " << param.genome_seq_file << "\n";
            OUT << "\t bases: " << param.bases << "\n\n";
        }

        OUT << "\t sink_index: " << param.sink_index << "\n";
        
        OUT << "\t enrich_method: ";
        if(param.smartSHAPE_enrich_method == SUBSTRACTION)
            OUT << "substraction" << "\n";
        else if(param.smartSHAPE_enrich_method == DIVIDING)
            OUT << "dividing" << "\n";
        else if(param.smartSHAPE_enrich_method == COMPLEX)
            OUT << "complex" << "\n";
        else if(param.smartSHAPE_enrich_method == RT)
            OUT << "rt" << "\n";
        else if(param.smartSHAPE_enrich_method == LOG)
            OUT << "log" << "\n";


        OUT << "\t BD_ext: " << param.BD_ext << "\n\n";

    }

    OUT.close();
}

int main(int argc, char *argv[])
{

    clog << "read parameters..." << endl;
    General_Param param = read_param(argc, argv);
    if(not param)
    {
        if(param.mode == Unknown)
            print_usage(argc, argv);
        else if(param.mode == TrtCont)
            print_usage_icSHAPE();
        else if(param.mode == Trt)
            print_usage_smartSHAPE();

        exit(-1);
    }

    if(not param.noParam)
        record_param(param, argc, argv);

    MapStringT<JunctionArray> junctions;
    MapStringuLONG chr_size;

    clog << "load_chr_size..." << endl;
    load_chr_size(param.size_file, chr_size);

    if(param.junction_file.empty())
    {
        if(not param.no_sliding)
            cerr << RED << "Waning: no junction file given" << DEF << endl;
    }else{
        clog << "load_junctions..." << endl;
        load_junctions(param.junction_file, junctions, chr_size);
    }

    vector<bool> chr_mask;
    string chr_seq;
    bool use_mask = param.bases.empty() ? false : true;
    qFasta fasta;
    if(use_mask)
        fasta.load_fasta_file(param.genome_seq_file);

    ofstream OUT(param.out_file+".tmp", ofstream::out);
    check_output_handle(OUT, param.out_file);

    if(param.mode == TrtCont)
    {

        if(use_mask)
            OUT << "@ColNum 11\n@ChrID 1\n@Strand 2\n@ChrPos 3\n@Base 4\n@N_RT 5\n@N_BD 6\n@D_RT 7\n@D_BD 8\n@Shape 9\n@ShapeNum 10\n@WindowShape 11\n";
            //OUT << "@ColNum 9\n@ChrID 1\n@Strand 2\n@ChrPos 3\n@Base 4\n@N_RT 5\n@N_BD 6\n@Shape 7\n@ShapeNum 8\n@WindowShape 9\n";
        else
            OUT << "@ColNum 10\n@ChrID 1\n@Strand 2\n@ChrPos 3\n@N_RT 4\n@N_BD 5\n@D_RT 6\n@D_BD 7\n@Shape 8\n@ShapeNum 9\n@WindowShape 10\n";

        clog << "Enter ==> TrtCont <== calculation mode" << endl;

        clog << "Start to init input handles..." << endl;
        uLONG D_num = param.DFiles.size();
        uLONG N_num = param.NFiles.size();
        StringArray DNFfiles;
        DNFfiles.reserve( D_num+N_num );
        DNFfiles.insert( DNFfiles.end(), param.DFiles.begin(), param.DFiles.end() );
        DNFfiles.insert( DNFfiles.end(), param.NFiles.begin(), param.NFiles.end() );

        vector<ifstream *> i_vec;
        for(string fn: DNFfiles)
        {
            i_vec.push_back( new ifstream(fn, ifstream::in) );
            check_input_handle(*i_vec.back(), fn);
        }

        StringArray chr_ids(D_num+N_num);
        vector< vector<Map_Record> > record_array(D_num+N_num);

        // parameter
        icSHAPE_Param shape_param = build_icSHAPE_param(param);

        int index = 1;
        while(sync_chrs(i_vec, chr_ids, record_array))
        {
            const string chr_id = chr_ids.front();
            const string chr = chr_id.substr(0, chr_id.size()-1);
            const char strand = chr_id.back();

            if(use_mask and not fasta.has_chr(chr))
            {
                cerr << RED << "Warning: " << chr_id << " not found on genome file, skip it" << DEF << endl;
                continue;
            }

            if(chr_size.find(chr_id) == chr_size.end())
            {
                cerr << RED << "Warning: " << chr_id << " not found on size file, skip it" << DEF << endl;
                continue;
            }

            // 2. calculation
            clog << index++ <<  ". read_chr " << chr_id << "\tsite: " << chr_size.at(chr_id) << endl;

            if(junctions.find(chr_id) == junctions.end())
            {
                if(not param.no_sliding)
                    cerr << RED << "Warning: " << chr_id << " junction not found, init empty junctions" << DEF << endl;
                junctions[chr_id]; 
            }

            if(junctions[chr_id].size() > 0)
            {
                clog << "Start to build_junction_support" << endl;
                for(uLONG d=0; d<D_num; d++)
                    build_junction_support(record_array.at(d), junctions.at(chr_id));

                clog << "Start to combine_junction" << endl;
                combine_junction(junctions[chr_id]);

                clog << "Start to check_overlap" << endl;
                check_overlap(junctions[chr_id], chr_size.at(chr_id));
            }

            uLONG cSize = chr_size.at(chr_id);
            uIntArray n_rt(cSize+1, 0), n_bd(cSize+1, 0);
            uIntArray d_rt(cSize+1, 0), d_bd(cSize+1, 0);

            uLONG BD_ext = 0;
            uLONG binsize = 1000000;

            FloatArray score(cSize+1, null);

            //ofstream OUT("test");

            if(strand == '+')
            {
                clog << "Start to calc_chr_BDRT_Pos" << endl;
                for(uLONG d=0; d<D_num; d++)
                {
                    calc_chr_BDRT_Pos(d_bd, d_rt, record_array.at(d), junctions[chr_id], cSize, BD_ext, binsize);
                    //for(int i=0;l)
                }
                for(uLONG n=0; n<N_num; n++)
                    calc_chr_BDRT_Pos(n_bd, n_rt, record_array.at(n+D_num), junctions[chr_id], cSize, BD_ext, binsize);

                //clog << "Start to sliding_non_junction..." << endl;
                //sliding_non_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, POSITIVE, OUT, shape_param, chr_mask, chr_seq);

                //clog << "Start to sliding_junction..." << endl;
                //sliding_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, POSITIVE, OUT, shape_param, chr_mask, chr_seq);
            }
            else
            {
                clog << "Start to calc_chr_BDRT_Neg" << endl;
                for(uLONG d=0; d<D_num; d++)
                    calc_chr_BDRT_Neg(d_bd, d_rt, record_array.at(d), junctions[chr_id], cSize, BD_ext, binsize);
                for(uLONG n=0; n<N_num; n++)
                    calc_chr_BDRT_Neg(n_bd, n_rt, record_array.at(n+D_num), junctions[chr_id], cSize, BD_ext, binsize);

                //clog << "Start to sliding_non_junction..." << endl;
                //sliding_non_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, NEGATIVE, OUT, shape_param, chr_mask, chr_seq);

                //clog << "Start to sliding_junction..." << endl;
                //sliding_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, NEGATIVE, OUT, shape_param, chr_mask, chr_seq);
                
            }

            ////////////////////////////////////////////////////////////////////////
            //if(use_mask)
            //{
                STRAND s = strand=='+' ? POSITIVE : NEGATIVE;

                if(param.base_separate){
                    for(char c: param.bases)
                    {
                        clog << "Start to calculate base " << c << endl;
                        vector<char> v;
                        v.push_back(c);
                        if(use_mask)
                            bool success = build_chr_mask(fasta, chr, s, v, chr_mask, chr_seq);
                        sliding_non_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                        sliding_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                    }
                }else{
                    clog << "Start to calculate all base " << param.bases << endl;
                    if(use_mask)
                        bool success = build_chr_mask(fasta, chr, s, param.bases, chr_mask, chr_seq);
                    sliding_non_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                    sliding_junction( n_rt, d_rt, n_bd, d_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                }
            //}
            ////////////////////////////////////////////////////////////////////////

            clog << " Finish " << chr_id << endl;
        }

        for(uLONG i=0; i<i_vec.size(); i++)
        {
            i_vec[i]->close();
            delete i_vec[i];
        }
    }else{
        if(use_mask)
            OUT << "@ColNum 9\n@ChrID 1\n@Strand 2\n@ChrPos 3\n@Base 4\n@N_RT 5\n@N_BD 6\n@Shape 7\n@ShapeNum 8\n@WindowShape 9\n";
        else
            OUT << "@ColNum 8\n@ChrID 1\n@Strand 2\n@ChrPos 3\n@N_RT 4\n@N_BD 5\n@Shape 6\n@ShapeNum 7\n@WindowShape 8\n";

        clog << "Enter ==> Trt <== calculation mode" << endl;

        clog << "Start to init input handles..." << endl;
        
        uLONG N_num = param.NFiles.size();
        vector<ifstream *> i_vec;
        for(string fn: param.NFiles)
        {
            i_vec.push_back( new ifstream(fn, ifstream::in) );
            check_input_handle(*i_vec.back(), fn);
        }

        StringArray chr_ids(N_num);
        vector< vector<Map_Record> > record_array(N_num);

        // parameter
        //smartSHAPE_Param shape_param;
        smartSHAPE_Param shape_param = build_smartSHAPE_param(param);

        int index = 1;
        while(sync_chrs(i_vec, chr_ids, record_array))
        {
            const string chr_id = chr_ids.front();
            const string chr = chr_id.substr(0, chr_id.size()-1);
            const char strand = chr_id.back();

            if(use_mask and not fasta.has_chr(chr))
            {
                cerr << RED << "Warning: " << chr_id << " not found on genome file, skip it" << DEF << endl;
                continue;
            }

            if(chr_size.find(chr_id) == chr_size.end())
            {
                cerr << RED << "Warning: " << chr_id << " not found on size file, skip it" << DEF << endl;
                continue;
            }

            // 2. calculation
            clog << index++ <<  ". read_chr " << chr_id << "\t" << chr_size.at(chr_id) << endl;

            if(junctions.find(chr_id) == junctions.end())
            {
                if(not param.no_sliding)
                    cerr << RED << "Warning: " << chr_id << " junction not found, init a empty junction" << DEF << endl;
                junctions[chr_id];
            }

            if(junctions[chr_id].size() >= 1)
            {
                clog << "build_junction_support" << endl;
                for(uLONG n=0; n<N_num; n++)
                    build_junction_support(record_array.at(n), junctions.at(chr_id));

                clog << "combine_junction" << endl;
                combine_junction(junctions[chr_id]);

                clog << "check_overlap" << endl;
                check_overlap(junctions[chr_id], chr_size.at(chr_id));
            }

            uLONG cSize = chr_size.at(chr_id);
            uIntArray n_rt(cSize+1, 0), n_bd(cSize+1, 0);

            uLONG binsize = 1000000;
            FloatArray score(cSize+1, null);

            if(strand == '+')
            {
                clog << "Start to calc_chr_BDRT_Pos" << endl;
                for(uLONG n=0; n<N_num; n++)
                    calc_chr_BDRT_Pos(n_bd, n_rt, record_array.at(n), junctions[chr_id], cSize, shape_param.BD_ext, binsize);

                //clog << "Start to sliding_non_junction..." << endl;
                //sliding_non_junction( n_rt, n_bd, junctions[chr_id], score, chr, POSITIVE, OUT, shape_param, chr_mask, chr_seq);

                //clog << "Start to sliding_junction..." << endl;
                //sliding_junction( n_rt, n_bd, junctions[chr_id], score, chr, POSITIVE, OUT, shape_param, chr_mask, chr_seq);
            }
            else
            {

                clog << "Start to calc_chr_BDRT_Pos" << endl;
                for(uLONG n=0; n<N_num; n++)
                    calc_chr_BDRT_Neg(n_bd, n_rt, record_array.at(n), junctions[chr_id], cSize, shape_param.BD_ext, binsize);

                //clog << "Start to sliding_non_junction..." << endl;
                //sliding_non_junction( n_rt, n_bd, junctions[chr_id], score, chr, NEGATIVE, OUT, shape_param, chr_mask, chr_seq);

                //clog << "Start to sliding_junction..." << endl;
                //sliding_junction( n_rt, n_bd, junctions[chr_id], score, chr, NEGATIVE, OUT, shape_param, chr_mask, chr_seq);
            }


            ////////////////////////////////////////////////////////////////////////
            //if(use_mask)
            //{
                STRAND s = strand=='+' ? POSITIVE : NEGATIVE;

                if(param.base_separate){
                    for(char c: param.bases)
                    {
                        clog << "Start to calculate all base " << c << endl;
                        vector<char> v;
                        v.push_back(c);
                        if(use_mask)
                            bool success = build_chr_mask(fasta, chr, s, v, chr_mask, chr_seq);
                        sliding_non_junction( n_rt, n_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                        sliding_junction( n_rt, n_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                    }
                }else{
                    clog << "Start to calculate all base " << param.bases << endl;
                    if(use_mask)
                        bool success = build_chr_mask(fasta, chr, s, param.bases, chr_mask, chr_seq);
                    sliding_non_junction( n_rt, n_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                    sliding_junction( n_rt, n_bd, junctions[chr_id], score, chr, s, OUT, shape_param, chr_mask, chr_seq);
                }
            //}
            ////////////////////////////////////////////////////////////////////////


            clog << " Finish " << chr_id << endl;
        }

        for(uLONG i=0; i<i_vec.size(); i++)
        {
            i_vec[i]->close();
            delete i_vec[i];
        }
    }

    OUT.close();

    clog << "Start to sort gTab..." << endl;
    sort_gTab(param.out_file+".tmp", param.out_file);
    remove ( (param.out_file+".tmp").c_str() );

    if(not param.out_junc_file.empty())
    {
        clog << "write_junctions" << endl;
        write_junctions(junctions, param.out_junc_file);
    }

}

