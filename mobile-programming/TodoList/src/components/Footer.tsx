import { FooterProps } from "../types";

function Footer(props: FooterProps){

    return (
        <footer>
            <button onClick={() => props.handleFilterChange("pending")}>
                <i>P - </i>
                <span>Pendentes</span>
            </button>
        
            <button onClick={() => props.handleFilterChange("all")}>
                <i>T - </i>
                <span>Todas</span>
            </button>

            <button onClick={() => props.handleFilterChange("completed")}>
                <i>C - </i>
                <span>Concluidas</span>
            </button>
        </footer>
    )
}

export default Footer;