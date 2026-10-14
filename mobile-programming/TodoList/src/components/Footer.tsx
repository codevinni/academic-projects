import type { ReactNode } from "react";
import { FooterProps, Filter } from "../types";

function Footer({ handleFilterChange, currentFilter = "all" }: FooterProps){

    const filters: { id: Filter; label: string; icon: ReactNode }[] = [
        {
            id: "pending",
            label: "Tarefas Pendentes",
            icon: (
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="10"></circle>
                    <polyline points="12 6 12 12 16 14"></polyline>
                </svg>
            )
        },
        {
            id: "all",
            label: "Todas as Tarefas",
            icon: (
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                    <line x1="8" y1="6" x2="21" y2="6"></line>
                    <line x1="8" y1="12" x2="21" y2="12"></line>
                    <line x1="8" y1="18" x2="21" y2="18"></line>
                    <circle cx="3.5" cy="6" r="1.5" fill="currentColor"></circle>
                    <circle cx="3.5" cy="12" r="1.5" fill="currentColor"></circle>
                    <circle cx="3.5" cy="18" r="1.5" fill="currentColor"></circle>
                </svg>
            )
        },
        {
            id: "completed",
            label: "Tarefas Concluídas",
            icon: (
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.4" strokeLinecap="round" strokeLinejoin="round">
                    <path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path>
                    <polyline points="22 4 12 14.01 9 11.01"></polyline>
                </svg>
            )
        }
    ];

    const activeIndex = filters.findIndex(f => f.id === currentFilter);
    const safeIndex = activeIndex >= 0 ? activeIndex : 1;

    return (
        <footer className="footer-dock-wrapper" aria-label="Filtros de tarefas">
            <nav className="footer-dock-bar">
                {/* Indicador deslizante com animação de transição lateral */}
                <div 
                    className="dock-slider-indicator" 
                    style={{ transform: `translateX(${safeIndex * 100}%)` }}
                    aria-hidden="true"
                />

                {filters.map((item) => {
                    const isActive = currentFilter === item.id;
                    return (
                        <button
                            key={item.id}
                            type="button"
                            onClick={() => handleFilterChange(item.id)}
                            className={`dock-filter-btn ${isActive ? 'dock-filter-btn--active' : ''}`}
                            aria-label={item.label}
                            title={item.label}
                        >
                            <span className="dock-btn-icon">
                                {item.icon}
                            </span>
                        </button>
                    );
                })}
            </nav>
        </footer>
    );
}

export default Footer;